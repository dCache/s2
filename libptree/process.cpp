#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "parse.h"
#include "n.h"			/* Node */
#include "process.h"		/* Process */
#include "expr.h"		/* Expression evaluation */
#include "printf.h"		/* $PRINTF{} */
#include "md5f.h"		/* md5 sum function */
#include "date.h"		/* $DATE{} */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"		/* BOOL, STD_BUF, ... */

#include "free.h"		/* FREE(), DELETE() */
#include "io.h"			/* file_ropen(), ... */
#include "s2.h"			/* opts (s2 options) */
#include "str.h"
#include "thread_pool.h"

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <signal.h>		/* signal() */
#include <stdlib.h>		/* exit(), system() */
#include <stdio.h>		/* stderr */
#include <errno.h>		/* errno */

#include <iostream>		/* std::string, cout, endl, ... */
#include <sstream>		/* ostringstream */

//#define USE_PTHREAD_ATTR	/* SEGVs! */

using namespace std;

#define UPDATE_MAX_MUTEX(mtx,m1,m2)\
  do {S_P(mtx); UPDATE_MAX(m1,m2); S_V(mtx);} while(0)

/* private data */
typedef struct threads {
  int total;
  pthread_mutex_t total_mtx;
  pthread_mutex_t evaluated_mtx;
  pthread_mutex_t print_mtx;
  pthread_cond_t timeout_cv;
  pthread_mutex_t timeout_mtx;
  pthread_attr_t attr;
} threads;
static struct threads thread;

typedef struct timeout_info_t {
  Process *p;
  BOOL terminated;
} timeout_info_t;

int
thread_create
(pthread_t *pthread, void *(*start_routine)(void *), void *arg)
{
  DM_DBG_I;
  int rval;

#ifdef USE_PTHREAD_ATTR /* SEGVs! */
  rval = pthread_create(pthread, &thread.attr, start_routine, arg);
#else
  rval = pthread_create(pthread, NULL, start_routine, arg);
#endif

  RETURN(rval);
}

int
thread_join
(pthread_t pthread, void **thread_return)
{
  DM_DBG_I;
  int rval;
  
  rval = pthread_join(pthread, thread_return);

  if(!rval) {
    /* thread successfully joined */
    DM_DBG(DM_N(3), "thread[%lu] successfully joined\n", pthread);
  }

  RETURN(rval);
}

/*
 * For use in threads only.
 */
void *
eval_in_parallel(void *proc)
{
  DM_DBG_I;

  Process *p = (Process *)proc;
  int root_eval;
  
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", p->n->row, p->executed, p->evaluated, p);
  DM_DBG(DM_N(3), "%s\n", p->n->toString(NULL).c_str());

  root_eval = p->eval_sequential_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, p->evaluated, root_eval);
  MUTEX(&thread.total_mtx, thread.total--);

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", p->n->row, p->executed, p->evaluated, pthread_self());
//  if(n->TIMEOUT) timeout_del(pthread_self());

#if 0
  pthread_exit((void *)proc);
#else
  return (void *)proc;
#endif
} /* eval_in_parallel */

#if 1
/*
 * function tp_handle_request(): handle a single given request.
 * algorithm: prints a message stating that the given thread handled
 *            the given request.
 * input:     request pointer
 * output:    none
 */
extern void *
tp_handle_request(void *request)
{
  DM_DBG_I;
  tp_request *p_request = (tp_request *)request;
  Process *p;
  
  if(p_request == NULL) {
    DM_ERR_ASSERT("request == NULL\n");
    RETURN(NULL);
  }
  p = (Process *)p_request->data;

  DM_DBG(DM_N(3), "proc=%p, thread (%d)\n", p, p_request->tp_tid);

  eval_in_parallel(p);

  DM_DBG(DM_N(3), "proc=%p, thread (%d): executed=%d; evaluated=%d\n",
         p, p_request->tp_tid, p->executed, p->evaluated);
  
  RETURN(NULL);
}
#endif

/*
 * Process constructor
 */
Process::Process()
{
  /* initialisation */
  init();
}

/*
 * Initialise Process
 */
void
Process::init()
{
  executed = ERR_OK;
  evaluated = ERR_OK;
}

/*
 * Initialise Process
 */
void
Process::init(Node *node, Process *p)
{
  n = node;

  /* TODO! */
  parent = p;
  rpar = NULL;
}

/*
 * Process constuctor
 */
Process::Process(Node *node, Process *p)
{
  Process::init(node, p);
  init();
}

/*
 * Process destructor
 */
Process::~Process()
{
  DELETE_VEC(vProc);		/* terminate finished processes */
}

int
Process::threads_init(void)
{
  DM_DBG_I;

  memset(&thread, 0, sizeof(threads));

#if USE_PTHREAD_ATTR
  pthread_attr_init(&thread.attr);
  pthread_attr_setstacksize(&thread.attr, THREAD_STACK_SIZE);
  pthread_attr_setdetachstate(&thread.attr, PTHREAD_CREATE_JOINABLE);
#endif

  pthread_mutex_init(&thread.total_mtx, NULL);
  pthread_mutex_init(&thread.evaluated_mtx, NULL);
  pthread_mutex_init(&thread.print_mtx, NULL);
  pthread_cond_init(&thread.timeout_cv, NULL);
  pthread_mutex_init(&thread.timeout_mtx, NULL);

  RETURN(ERR_OK);
}

int
Process::threads_destroy(void)
{
#if USE_PTHREAD_ATTR
  pthread_attr_destroy(&thread.attr);   /* valgrind complains */
#endif

  pthread_mutex_destroy(&thread.total_mtx);
  pthread_mutex_destroy(&thread.evaluated_mtx);
  pthread_mutex_destroy(&thread.print_mtx);
  pthread_cond_destroy(&thread.timeout_cv);
  pthread_mutex_destroy(&thread.timeout_mtx);

  return ERR_OK;
}

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 * 
 * Take care of parallel branches.
 */
int
Process::eval()
{
  DM_DBG_I;
  std::string str_node = Node::nodeToString(n, n->OFFSET, this);
  DM_DBG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_LOG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", n->row, executed, evaluated, this);

  int root_eval;
  int sreqs = 0;	/* number of parallel subrequests created */
  pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;

  /* Schedule parallel execution */
  if(n->par && n->COND == S2_COND_NONE) {
    Node *ptr_node;
    /* S2_COND_NONE is important not to schedule parallel execution more than once *
     * e.g. when evaluating OR or AND branches                                     */

    DM_DBG(DM_N(3), FBRANCH"branches located at the same offset exist\n", n->row, executed, evaluated);

    /* check for parallel execution and create new requests if parallel branches found */
    for(ptr_node = n->par; ptr_node; ptr_node = ptr_node->par) {
      BOOL can_enqueue;
      can_enqueue = FALSE;

      if(ptr_node->COND == S2_COND_NONE) {
        if(ptr_node->REPEAT.type == S2_REPEAT_PAR) {
          /* repeats execution */
          int repeats_eval;
          int8_t step = n->REPEAT.X < n->REPEAT.Y? 1 : -1;
          int64_t i = n->REPEAT.X - step;

          do {
            i += step;
            S_P(&thread.total_mtx);
            if(thread.total < opts.tp_size) {
              thread.total++;
              can_enqueue = TRUE;
            }
            S_V(&thread.total_mtx);

            if(can_enqueue) {
              Process *proc = new Process(ptr_node, this);
              proc->I = i;
              vProc.push_back(proc);
              DM_DBG(DM_N(1), "parallel repeat branch %u: enqueing a request\n", ptr_node->row);
              if(tp_enqueue(proc, &sreqs, &sreqs_cv)) {
                DM_ERR(ERR_SYSTEM, _("branch %u: failed to create new thread: %s\n"), ptr_node->row, strerror(errno));
              } else {
                /* number of sub-requests */
                sreqs++;
                DM_DBG(DM_N(3), "branch %u: subrequests=%d\n", ptr_node->row, sreqs);
              }
            } else {
              /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
                 => evaluate sequentially */
              Process proc = Process(ptr_node, this);
              proc.I = i;
              DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", ptr_node->row, thread.total, proc.executed, proc.evaluated, opts.tp_size);
              repeats_eval = proc.eval_with_timeout();
              UPDATE_MAX_MUTEX(&thread.evaluated_mtx, proc.evaluated, repeats_eval);
            }
          } while(i != n->REPEAT.Y);

          continue;
        }
        
        S_P(&thread.total_mtx);
        if(thread.total < opts.tp_size) {
          thread.total++;
          can_enqueue = TRUE;
        }
        S_V(&thread.total_mtx);
        
        if(can_enqueue) {
          Process *proc = new Process(ptr_node, this);
          vProc.push_back(proc);
          DM_DBG(DM_N(1), FBRANCH"enqueing a request\n", n->row, executed, evaluated);
          if(tp_enqueue(proc, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, executed, evaluated, sreqs);
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          Process proc = Process(ptr_node, this);
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, executed, evaluated, thread.total, opts.tp_size);
          root_eval = proc.eval_repeats();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, root_eval);
        }
      } /* if */
    } /* for */
  } else {
    DM_DBG(DM_N(3), FBRANCH"no branches located at the same offset exist\n", n->row, executed, evaluated);
  }

  root_eval = eval_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, root_eval);
  DM_DBG(DM_N(3), FBRANCH"root_eval=%d\n", n->row, executed, evaluated, root_eval);

  S_P(&sreqs_mtx);
  while(sreqs != 0) {
    DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", n->row, executed, evaluated, sreqs);
    pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
  }
  S_V(&sreqs_mtx);

  /* threads-related cleanup */
  pthread_cond_destroy(&sreqs_cv);
  pthread_mutex_destroy(&sreqs_mtx);

  /* Parallel execution finished, we have one thread of execution. *
   * Go through the parallel branches and set the return value.    */
  if(n->par) {
    std::vector<Process *>::const_iterator it;
    for (it = vProc.begin(); it != vProc.end(); it++) {
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, (*it)->evaluated);
    }
  }

  DM_DBG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);
  DM_LOG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);

  if(opts.e2_fname) Node::print_node(n, n->OFFSET, opts.e2_file, this, TRUE, TRUE);

  RETURN(evaluated);
}

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Process::eval_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", n->row, executed, evaluated, this);

  int repeats_eval;

  switch(n->REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_OR:		/* fall through */
    case S2_REPEAT_AND:		/* fall through */
seq:
      I = n->REPEAT.X;		/* necessary for S2_REPEAT_NONE only */
      repeats_eval = eval_sequential_repeats();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, repeats_eval);
      DM_DBG(DM_N(5), FBRANCH"repeats_eval=%d\n", n->row, executed, evaluated, repeats_eval);
    break;

    case S2_REPEAT_PAR:
      int8_t step = n->REPEAT.X < n->REPEAT.Y? 1 : -1;
      int64_t i = n->REPEAT.X - step;
      uint threads_total = 1 + ((n->REPEAT.X > n->REPEAT.Y)? n->REPEAT.X - n->REPEAT.Y: n->REPEAT.Y - n->REPEAT.X);
      if(threads_total == 1)
        /* no parallel threads needed */
        goto seq;

      int sreqs = 0;		/* number of parallel subrequests created */
      pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
      pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;
      DM_DBG(DM_N(1), _("parallel repeat "FBRANCH"total number of threads=%d\n"), n->row, executed, evaluated, threads_total);
      do {
        i += step;
        BOOL can_enqueue;
        can_enqueue = FALSE;
        
        S_P(&thread.total_mtx);
        if(thread.total < opts.tp_size) {
          thread.total++;
          can_enqueue = TRUE;
        }
        S_V(&thread.total_mtx);

        if(can_enqueue) {
          DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", n->row, executed, evaluated);
          Process *proc = new Process(n, this);
          proc->I = i;
          vProc.push_back(proc);
          if(tp_enqueue(proc, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, executed, evaluated, sreqs);
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          I = i;
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, executed, evaluated, thread.total, opts.tp_size);
          repeats_eval = eval_with_timeout();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, repeats_eval);
        }
      } while(i != n->REPEAT.Y);

      S_P(&sreqs_mtx);
      while(sreqs != 0) {
        DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", n->row, executed, evaluated, sreqs);
        pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
      }
      S_V(&sreqs_mtx);

      /* threads-related cleanup */
      pthread_cond_destroy(&sreqs_cv);
      pthread_mutex_destroy(&sreqs_mtx);

    break;
  }

  DM_DBG(DM_N(5), FBRANCH"eval_repeats returns\n", n->row, executed, evaluated);
  RETURN(evaluated);
} /* eval_repeats */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Process::eval_sequential_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", n->row, executed, evaluated, this);

  int8_t step = n->REPEAT.X < n->REPEAT.Y? 1 : -1;
  int64_t i = n->REPEAT.X - step;
  int iter_eval;

  switch(n->REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_PAR:		/* fall through: parallelism already handled in eval_repeats() */
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"\n", n->row, executed, evaluated);
      iter_eval = eval_with_timeout();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);
    break;

    case S2_REPEAT_OR:
      I = i;
      do {
        I += step;
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", n->row, evaluated, executed, I, n->REPEAT.Y);

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"iter_eval=%d\n", n->row, evaluated, executed, iter_eval);
        if(evaluated <= n->EVAL) {
          DM_DBG(DM_N(5), "OR repeat "FBRANCH"successfully evaluated; evaluated=%d <= EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first successful evaluation */
          break;
        }
      } while(I != n->REPEAT.Y);
    break;

    case S2_REPEAT_AND:
      I = i;
      do {
        I += step;
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", n->row, executed, evaluated, I, n->REPEAT.Y);

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);
        if(evaluated > n->EVAL) {
          DM_DBG(DM_N(5), "AND repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first unsuccessful evaluation */
          break;
        }
      } while(I != n->REPEAT.Y);
    break;

  }

  DM_DBG(DM_N(5), FBRANCH"eval_sequential_repeats returns\n", n->row, executed, evaluated);
  RETURN(evaluated);
} /* eval_sequential_repeats */

/*
 * function pthread_timeout_handler(): free thread-related data.
 * input:     pointer to the node.
 * output:    none
 */
static void
pthread_timeout_handler(void *proc)
{
  DM_DBG_I;
  pthread_t tid = pthread_self();	/* thread identifying number */
  Process *p = (Process *)proc;

  DM_DBG(DM_N(3), FBRANCH"cleaning up thread (%lu)\n", p->n->row, p->executed, p->evaluated, tid);

  S_V(&thread.total_mtx);
  S_V(&thread.evaluated_mtx);
  S_V(&thread.print_mtx);

  DM_DBG_O;
}

/*
 * For use in threads only.
 */
void *
exec_in_parallel_without_timeout(void *timeout_info)
{
  DM_DBG_I;

  timeout_info_t *ti = (timeout_info_t *)timeout_info;
  int root_eval;
  
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", ti->p->n->row, ti->p->executed, ti->p->evaluated, ti->p->n);
  DM_DBG(DM_N(3), "%s\n", ti->p->n->toString(FALSE).c_str());

  root_eval = ti->p->n->exec(ti->p);
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, ti->p->evaluated, root_eval);
  ti->terminated = TRUE;

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", ti->p->n->row, ti->p->executed, ti->p->evaluated, pthread_self());
  
  if(pthread_cond_broadcast(&thread.timeout_cv)) {
    DM_ERR(ERR_SYSTEM, _("pthread_cond_signal failed: %s\n"), strerror(errno));
  }

#if 0
  pthread_exit((void *)ti->p);
#else
  return (void *)ti->p;
#endif
} /* exec_in_parallel_without_timeout */

/*
 * Execute the current process with timeout if any.
 */
int
Process::exec_with_timeout()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", n->row, executed, evaluated, this);

  pthread_t thread_id;
  struct timeval now;			/* timeval uses micro-seconds */
  struct timespec timeout;		/* timespec uses nano-seconds */
  timeout_info_t ti;

  if(!n->TIMEOUT) {
    /* no timeout needed */
    DM_DBG(DM_N(3), FBRANCH"no timeout set\n", n->row, executed, evaluated);
    RETURN(n->exec(this));
  }

  /* set thread cleanup handler */
  DM_DBG(DM_N(3), FBRANCH"pushing cleanup handler\n", n->row, executed, evaluated);
  pthread_cleanup_push(pthread_timeout_handler, (void*)this);

  ti.p = this;
  ti.terminated = FALSE;
  
  /* timeouts handling */
  DM_DBG(DM_N(3), FBRANCH"proc=%p; creating new thread with timeout=%"PRIu64"\n", n->row, executed, evaluated, this, n->TIMEOUT);
  int thread_rval = thread_create(&thread_id, exec_in_parallel_without_timeout, &ti);
  if(thread_rval)
  {
    DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create thread: %s\n"), n->row, executed, evaluated, strerror(errno));
  } else {
    /* thread was created fine */
    DM_DBG(DM_N(3), FBRANCH"thread %lu created successfully, adding timeout=%"PRIu64"\n", n->row, executed, evaluated, thread_id, n->TIMEOUT);
    
    if(gettimeofday(&now, NULL)) {
      DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
      RETURN(ERR_SYSTEM);
    }
    unsigned long timeout_add_sec = n->TIMEOUT / 1000000;
    unsigned long timeout_add_usec = n->TIMEOUT - ((n->TIMEOUT / 1000000) * 1000000);

    timeout.tv_sec = now.tv_sec + timeout_add_sec;
    timeout.tv_nsec = (now.tv_usec + timeout_add_usec) * 1000;

    S_P(&thread.timeout_mtx);
    DM_DBG_T(DM_N(2), FBRANCH"waiting for timeout_cv\n", n->row, executed, evaluated);
    int rc;
    while(1) {
      rc = pthread_cond_timedwait(&thread.timeout_cv, &thread.timeout_mtx, &timeout);
      if(rc == ETIMEDOUT) {
        /* timeout reached, cancel the thread */
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv timed out\n", n->row, executed, evaluated);
        break;
      }
      if(ti.terminated) {
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv triggered\n", n->row, executed, evaluated);
        break;
      }
    }
    S_V(&thread.timeout_mtx);

    if(rc == ETIMEDOUT) {
      /* timeout reached, cancel the thread */
      DM_DBG_T(DM_N(2), FBRANCH"cancelling thread %lu\n", n->row, executed, evaluated, thread_id);
      pthread_cancel(thread_id);
    }
  }

  /* ``reap'' the thread */
  Process *p;	/* pointer to a return value from a thread */
  DM_DBG(DM_N(3), "reaping thread %lu\n", n->row, executed, evaluated, thread_id);
  if(thread_join(thread_id, (void **)&p)) {
    DM_ERR(ERR_SYSTEM, FBRANCH"failed to join thread: %s\n", n->row, executed, evaluated, strerror(errno));
  }
  DM_DBG(DM_N(3), FBRANCH"joined thread; proc ptr=%p\n", n->row, executed, evaluated, p);
  if(p == PTHREAD_CANCELED) {
    /* timeout */
    int root_eval;
    DM_DBG(DM_N(2), FBRANCH"timeout reached\n", n->row, executed, evaluated);
    DM_LOG(DM_N(2), FBRANCH"timeout reached\n", n->row, executed, evaluated);
    root_eval = ERR_NEXEC;
    UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, ERR_NEXEC);
  }

  /* Remove thread cleanup handler. */
  pthread_cleanup_pop(0);

  RETURN(evaluated);
} /* exec_with_timeout */

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_with_timeout()
{
  int timeout_exec, timeout_eval;

  if(opts.e0_fname) Node::print_node(n, n->OFFSET, opts.e0_file, this, FALSE, FALSE);

  DM_DBG_T(DM_N(4), FBRANCH"starting execution of proc=%p\n", n->row, executed, evaluated, this);
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(this, OFFSET, this).c_str());
  timeout_exec = exec_with_timeout();
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(this, OFFSET, this).c_str());
  S_P(&thread.evaluated_mtx);
  executed = evaluated = timeout_exec;	/* for ${?} */
  S_V(&thread.evaluated_mtx);
  if(opts.e1_fname) Node::print_node(n, n->OFFSET, opts.e1_file, this, TRUE, FALSE);
  DM_DBG_T(DM_N(4), FBRANCH"finished execution (of proc=%p)\n", n->row, executed, evaluated, this);
  DM_LOG(DM_N(2), FBRANCH"executed=%d\n", n->row, executed, evaluated, timeout_exec);

  timeout_eval = eval_subtree(timeout_exec, timeout_exec);
  DM_DBG(DM_N(4), FBRANCH"proc=%p; timeout_eval=%d\n", n->row, executed, evaluated, this, timeout_eval);

  S_P(&thread.evaluated_mtx);
  evaluated = timeout_eval;
  S_V(&thread.evaluated_mtx);

  return timeout_eval;
}

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_subtree(const int root_exec, int &root_eval)
{
  int child_eval, par_eval; 

  /* investigate CHILDREN */
  if(root_exec <= n->EVAL) { 
    /* evaluate the children (only if root executed fine) */
    if(n->child) {
      DM_DBG(DM_N(3), FBRANCH"evaluating child branch %u\n", n->row, executed, evaluated, n->child->row);
      Process proc = Process(n->child, this);
      child_eval = proc.eval();
      root_eval = child_eval;
    }
  }

  DM_DBG(DM_N(5), FBRANCH"subtree_eval=%d\n", n->row, executed, evaluated, root_eval);

  /* Investigate branches at the same offset */
  if(n->par) {
    Node *ptr_node = n->par;
    DM_DBG(DM_N(5), FBRANCH"investigating branches located at the same offset\n", n->row, executed, evaluated);
    switch(n->par->COND) {
      case S2_COND_OR:
        if(root_eval > n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"OR: root_eval=%d > EVAL=%d\n", n->row, executed, evaluated, root_eval, n->EVAL);
          Process proc = Process(n->par, this);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        } else {
          /* see if we have an AND node that we need to evaluate */
          DM_DBG(DM_N(5), FBRANCH"OR: evaluated(%d) <= EVAL(%d)\n", n->row, executed, evaluated, evaluated, n->EVAL);

          ptr_node = n->par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_AND) goto eval_and;
            ptr_node = ptr_node->par;
          }
          break;

eval_and:
          DM_DBG(DM_N(5), FBRANCH"found a par AND, evaluating branch %u\n", n->row, executed, evaluated, ptr_node->row);
          Process proc = Process(ptr_node, this);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        }
      break;

      case S2_COND_AND:
        if(root_eval <= n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) <= EVAL(%d)\n", n->row, executed, evaluated, root_eval, n->EVAL);
          Process proc = Process(n->par, this);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        } else {
          /* see if we have an OR node that can save us (return TRUE evaluation) */
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) > EVAL(%d)\n", n->row, executed, evaluated, root_eval, n->EVAL);

          ptr_node = n->par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_OR) goto eval_or;
            ptr_node = ptr_node->par;
          }
          break;

eval_or:
          DM_DBG(DM_N(5), FBRANCH"found a par OR, evaluating branch %u\n", n->row, executed, evaluated, ptr_node->row);
          Process proc = Process(ptr_node, this);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        }
      break;
      
      case S2_COND_NONE:
        /* parallel execution, taken care of by a new thread scheduled at the top */
        DM_DBG(DM_N(5), FBRANCH"found a parallel branch, execution already scheduled\n", n->row, evaluated, executed);
      break;
    }
  }

  return root_eval;
}
/* eval_subtree */

std::string
Process::toString()
{
  std::stringstream ss;

  ss << "Process";

  return ss.str();
}

/*
 * Evaluate the expected string before PCRE matching.
 */
BOOL
Process::e_match(const std::string *expected, const char *received)
{
  BOOL match;
  std::string s_expected;

  if(expected == NULL || received == NULL) {
    DM_DBG(DM_N(1), "expected == %p || received == %p\n", expected, received);
    RETURN(!(n->match_opt.pcre & PCRE_NOTEMPTY));
  }

  s_expected = eval_str(expected, this);
  match = pcre_matches(s_expected.c_str(), received, n->match_opt.pcre, WriteVariable);
  
  return match;
} /* e_match */

BOOL
Process::e_match(const char *expected, const char *received)
{
  BOOL match;
  std::string s_expected = eval_str(expected, this);
  match = pcre_matches(s_expected.c_str(), received, n->match_opt.pcre, WriteVariable);

  return match;
} /* e_match */

/*
 * Try to evaluate an expression to a 64-bit integer.
 * 
 * Returns: 0 on success
 *          1 on failure and e is not modified
 */
extern int
str_expr2i(const char *cstr, int64_t *e)
{
  Expr expr(cstr);
  return expr.parse(e);
}

/*
 * Evaluate cstr and return evaluated std::string.
 */
std::string
Process::eval_str(const char *cstr, Process *proc)
{
  int i, c;
  int slen;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  std::string s;
  std::string var;
  enum s_eval { 
    sInit, sDollar, sVar, sEvar, sCounter, sExpr,  sRnd,  sDate,  sPrintf,
    sMd5,
  } state = sInit;
  static const char* state_name[] = {
    "0",   "",      "",   "ENV", "I",      "EXPR", "RND", "DATE", "PRINTF",
    "MD5", 
  };
  const char *opt;
  int opt_off;
  int brackets = 0;

  if(cstr == NULL) {
    DM_ERR_ASSERT(_("c_str == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  if(!proc) return std::string(cstr);

  s.clear();
  slen = strlen(cstr);
  for(i = 0; i < slen; i++) {
//    DM_DBG(DM_N(0), "------>%d\n", i);
    c = cstr[i];
    opt = cstr + i;
    if(c == '{') brackets++;
    if(c == '}') brackets--;
//    DM_DBG(DM_N(5), "brackets=%d\n", brackets);

    switch (state) {
      case sInit:{
        if(c == '$') {
          if(!bslash) {
            state = sDollar;
            continue;
          }
        }
      }
      break;

      case sDollar:{
        if(c == '{') {
          /* we have a reference to a variable */
          var.clear();
          state = sVar;
          continue;
        } else if(OPL("ENV{")) {
          /* we have a reference to environment variable */
          var.clear();
          state = sEvar;
          i += opt_off - 1;
        } else if(OPL("I{")) {
          /* we have a reference to a repeat loop counter */
          var.clear();
          state = sCounter;
          i += opt_off - 1;
        } else if(OPL("EXPR{")) {
          /* expression evaluation */
          var.clear();
          state = sExpr;
          i += opt_off - 1;
        } else if(OPL("RND{")) {
          /* random values */
          var.clear();
          state = sRnd;
          i += opt_off - 1;
        } else if(OPL("DATE{")) {
          /* date/time */
          var.clear();
          state = sDate;
          i += opt_off - 1;
        } else if(OPL("PRINTF{")) {
          /* date/time */
          var.clear();
          state = sPrintf;
          i += opt_off - 1;
        } else if(OPL("MD5{")) {
          /* MD5 sum of a string */
          var.clear();
          state = sMd5;
          i += opt_off - 1;
        } else {
          /* return the borrowed dollar */
          s.push_back('$');
          state = sInit;
          break;
        }
        brackets++;
      }
      continue;

      case sVar:{
        if(c == '}' && !brackets) {
          /* we have a complete variable */
          if(var == "?") {	/* ${?} */
            /* return parent's execution value */
            int parent_executed = proc->parent? proc->parent->executed: 0;
            DM_DBG(DM_N(4), "returning parent's execution value %d\n", parent_executed);
            s.append(i2str(parent_executed));
            state = sInit;
            continue;
          }
          if(var == "!") {	/* ${!} */
            /* return evaluation value of the branch at the same offset */
            int rpar_evaluated = proc->rpar? proc->rpar->evaluated: 0;
            DM_DBG(DM_N(4), "returning rpar evaluation value %d\n", rpar_evaluated);
            s.append(i2str(rpar_evaluated));
            state = sInit;
            continue;
          }
          /* classic variable ${var} */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: ${...${var}...} */

          const char *new_var;
          const char *read_var;
          if(var[0] == '-')
            /* issue no warnings when a variable is undefined */
	    read_var = var.substr(1).c_str();
          else read_var = var.c_str();

          DM_DBG(DM_N(4), "read_var=|%s|\n", read_var);
          if((new_var = ReadVariable(read_var)) != NULL) {
            DM_DBG(DM_N(4), "var %s=|%s|\n", var.c_str(), new_var);
            s.append(new_var);
          } else {
            /* the variable is not defined, output its name */
            if(var[0] != '-') {
              DM_WARN(ERR_WARN, _("variable `%s' is unset\n"), var.c_str());
            }
            s.append("${" + std::string(read_var) + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sEvar:{
        if(c == '}' && !brackets) {
          /* we have a complete environment variable */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: $ENV{...${var}...} */

          const char *env_var;
          env_var = getenv(var.c_str());
          
          if(env_var) {
            DM_DBG(DM_N(4), "env var %s=|%s|\n", var.c_str(), env_var);
            s.append(env_var);
          } else {
            /* the environment variable is not defined, output its name */
            DM_WARN(ERR_WARN, _("environment variable `%s' is unset\n"), var.c_str());
            s.append("$ENV{" + var + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sCounter:{
        if(c == '}' && !brackets) {
          /* we have a reference to a repeat loop counter */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: $I{...${var}...} */

          const char *word;
          word = var.c_str();
          char *endptr;
          int16_t i16;

          i16 = get_int16(word, &endptr, FALSE);
          if(endptr != word) {
            int64_t i64 = 0;
            Process *ptr_proc = proc;
            while(ptr_proc && i16 >= 0) {
              if(ptr_proc->n->REPEAT.type == S2_REPEAT_OR || 
                 ptr_proc->n->REPEAT.type == S2_REPEAT_AND)
              {
                DM_DBG(DM_N(4), "found OR or AND repeat; i16=%d\n", i16);
                if(!i16) {
                  i64 = ptr_proc->I;
                  DM_DBG(DM_N(4), "$I{%d}=%"PRIi64"\n", i16, i64);
                  break;
                } 
                i16--;
              }
              ptr_proc = ptr_proc->parent;
            }
            if(ptr_proc == NULL)
              DM_WARN(ERR_WARN, _(FBRANCH"couldn't find repeat loop for repeat counter $I{%s}!\n"), proc->n->row, proc->executed, proc->evaluated, word);

            DM_DBG(DM_N(4), "writing repeat counter value: %"PRIi64"\n", i64);
            s.append(i2str(i64));
          } else {
            DM_WARN(ERR_WARN, _("cannot evaluate loop nesting `%s': %s\n"), word, _(strerror(errno)));
            s.append("$I{" + var + "}");
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sExpr:{
        if(c == '}' && !brackets) {
          /* we have a complete expression to evaluate */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: $EXPR{...${var}...} */

          int64_t e = 0;
          DM_DBG(DM_N(4), "expr=|%s|\n", var.c_str());
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), proc->n->row, proc->executed, proc->evaluated, var.c_str());
          } else s.append(i2str(e));
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sRnd:{
        if(c == '}' && !brackets) {
          /* we have a maximum+1 random number */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: $RND{...${var}...} */

          int64_t e = 0;
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), proc->n->row, proc->executed, proc->evaluated, var.c_str());
          } else {
            /* srandom() is done elsewhere */
            s.append(i2str(random() % e));
          }
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sDate:{
        if(c == '}' && !brackets) {
          struct timeval timestamp;
          struct tm *now;
          std::string date;
        
          gettimeofday(&timestamp, NULL);
          now = localtime(&(timestamp.tv_sec));

          if(var == "") {
            /* use the default format */
            date = ssprintf("%04d-%02d-%02d@%02d:%02d:%02d.%06lu",
                     now->tm_year+1900, now->tm_mon+1, now->tm_mday,
                     now->tm_hour, now->tm_min, now->tm_sec, timestamp.tv_usec);
          } else {
            int j, esc = 0, len = var.length();
            for(j = 0; j < len; j++) {
              if(!esc) {
                if(var[j] == '%') esc = 1;
                else s.push_back(var[j]);
                continue;
              }

              /* escape */
              esc = 0;
              switch(var[j]) {
                case 'C': s.append(ssprintf("%02d", (now->tm_year+1900)/100)); break;
                case 'D': s.append(ssprintf("%02d/%02d/%02d", now->tm_mon+1, now->tm_mday, now->tm_year % 100)); break;
                case 'd': s.append(ssprintf("%02d", now->tm_mday)); break;
                case 'e': s.append(ssprintf("%2d", now->tm_mday)); break;
                case 'F': s.append(ssprintf("%04d-%02d-%02d", now->tm_year+1900, now->tm_mon+1, now->tm_mday)); break;
                case 'H': s.append(ssprintf("%02d", now->tm_hour)); break;
                case 'k': s.append(ssprintf("%2d", now->tm_hour)); break;
                case 'I': s.append(ssprintf("%02d", (now->tm_hour % 12) ? (now->tm_hour % 12) : 12)); break;
                case 'l': s.append(ssprintf("%2d", (now->tm_hour % 12) ? (now->tm_hour % 12) : 12)); break;
                case 'j': s.append(ssprintf("%02d", now->tm_yday+1)); break;
                case 'M': s.append(ssprintf("%02d", now->tm_min)); break;
                case 'm': s.append(ssprintf("%02d", now->tm_mon+1)); break;
                case 'n': s.push_back('\n'); break;
                case 'N': s.append(ssprintf("%06d", timestamp.tv_usec)); break; /* cut to 6 digits */
                case 'R': s.append(ssprintf("%02d:%02d", now->tm_hour, now->tm_min)); break;
                case 's': s.append(ssprintf("%d", timestamp.tv_sec)); break;
                case 'S': s.append(ssprintf("%02d", now->tm_sec)); break;
                case 't': s.push_back('\t'); break;
                case 'u': s.append(ssprintf("%d", now->tm_wday)); break;
                case 'V': s.append(ssprintf("%d", week(now))); break;
                case 'T': s.append(ssprintf("%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec)); break;
                case 'Y': s.append(ssprintf("%04d", now->tm_year+1900)); break;
                case 'y': s.append(ssprintf("%02d", now->tm_year%100)); break;
                case 'w': s.append(ssprintf("%d", now->tm_wday-1)); break;
                default:
                  s.push_back(var[j]);
              }
            }
            if(esc) s.push_back('%');
          }
          s.append(date);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sPrintf:{
        if(c == '}' && !brackets) {
          char **argv;
          int spaces = 0;
          int j, plen;
          const char *var_cstr = NULL;

          var = eval_str(var.c_str(), proc);	/* evaluate things like: $PRINTF{...${var}...} */
          var_cstr = var.c_str();
          plen = var.length();
          for(j = 0; j < plen; j++) {
            /* count the number of spaces to guess the maximum number of $PRINTF parameters (-2) */
            if(IS_WHITE(var_cstr[j])) spaces++;
          }
          if((argv = (char **)malloc(sizeof(char **) * (spaces + 2))) == (char **)NULL) {
            DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
            return s;
          }
          std::string target;
          int l = 0;
          for(j = 0;; j++) {
            int chars;
            chars = get_dq_param(target, var_cstr + l);
            if(chars == 0) break;
            if((argv[j] = (char *)strdup(target.c_str())) == (char *)NULL) {
              DM_ERR(ERR_SYSTEM, _("strdup failed\n"));
              return s;
            }
            l += chars;
          }
          argv[j] = 0;

          s.append(ssprintf_chk(argv));
          for(;j >= 0; j--) {
            FREE(argv[j]);
          }
          FREE(argv);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sMd5:{
        if(c == '}' && !brackets) {
          /* we have a maximum+1 random number */
          var = eval_str(var.c_str(), proc);	/* evaluate things like: $MD5{...${var}...} */
          char md5str[33];
          gen_md5(md5str, var.c_str());
          s.append(md5str);
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

    }

    /* no TAGs found, simply copy the characters */
    s.push_back(c);

    if(c == '\\') {
      if(bslash) {
        /* two backslashes => no quoting */
        bslash = FALSE;
      } else {
        bslash = TRUE;
      }
    }
  }

  /* sanity checks */
  if(state == sDollar) {
    s.push_back('$');
  }
  
  if(state != sInit) {
    DM_WARN(ERR_WARN, _("unterminated $%s{\n"), state_name[state]);
    s.append("$" + std::string(state_name[state]) + "{" + var);
  }
  
  return s;
} /* eval_str */

std::string
Process::eval_str(const std::string *s, Process *proc)
{
  if(s == NULL) {
    DM_ERR_ASSERT(_("s == NULL\n"));
    return std::string(S2_NULL_STR);
  }
  return eval_str(s->c_str(), proc);
} /* eval_str */

#define _EVAL2INT(u,s)\
u##int##s##_t \
Process::eval2##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
\
  if(str == NULL) {\
    DM_DBG(DM_N(0), _("returning default value 0"));\
    return 0;\
  }\
  str_val = eval_str(str, this);\
\
  return get_##u##int##s(str_val.c_str(), &endptr, TRUE);\
}
_EVAL2INT(,32);
_EVAL2INT(,64);
_EVAL2INT(u,32);
_EVAL2INT(u,64);
#undef _EVAL2INT

#define _EVAL2PINT(u,s)\
p##u##int##s##_t \
Process::eval2p##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
  p##u##int##s##_t r;\
\
  if(str == NULL) goto ret_null;\
  str_val = eval_str(str, this);\
\
  r.v = get_##u##int##s(str_val.c_str(), &endptr, TRUE);\
  if(str_val.c_str() == endptr)\
    /* couldn't convert to int */\
    goto ret_null;\
\
  r.p = &r.v;\
  return r;\
\
ret_null:\
  r.p = NULL;\
  return r;\
}
_EVAL2PINT(,32);
_EVAL2PINT(,64);
_EVAL2PINT(u,32);
_EVAL2PINT(u,64);
#undef _EVAL2PINT

/*
 * Evaluate std::vector <std::string *>.
 */
std::vector <std::string *>
Process::eval_vec_str(const std::vector <std::string *> &v)
{
  std::vector <std::string *> ev;

  for(uint i = 0; i < v.size(); i++) {
    if(v[i]) ev.push_back(new std::string(Process::eval_str(v[i], this).c_str()));
    else ev.push_back((std::string *)NULL);
  }

  return ev;
} /* eval_vec_str */

/*
 * Evaluate std::vector <std::string *>.
 */
#define _EVAL_VEC_INT(u,s,fp)\
std::vector <u##int##s##_t> \
Process::eval_vec_##u##int##s(const std::vector <std::string *> &v)\
{\
  std::vector <u##int##s##_t> ev;\
\
  for(uint c = 0; c < v.size(); c++) {\
    if(v[c]) {\
      u##int##s##_t i = eval2##u##int##s(v[c]);\
      DM_DBG(DM_N(3), "evaluating[%u] to %"fp"\n", c, i);\
      ev.push_back(i);\
    } else {\
      DM_DBG(DM_N(3), "default evaluation[%u] to 0\n", c);\
      ev.push_back(0);\
    }\
  }\
\
  return ev;\
}
_EVAL_VEC_INT(,32,"ld");
_EVAL_VEC_INT(,64,"lld");
_EVAL_VEC_INT(u,32,"lu");
_EVAL_VEC_INT(u,64,"llu");
#undef _EVAL_VEC_INT

/*
 * Evaluate std::vector <std::string *> to a std::vector of pointers to integers.
 */
#define _EVAL_VEC_PINT(u,s,fp)\
std::vector <u##int##s##_t *> \
Process::eval_vec_p##u##int##s(const std::vector <std::string *> &v)\
{\
  std::vector <u##int##s##_t *> ev;\
\
  for(uint c = 0; c < v.size(); c++) {\
    u##int##s##_t *i = NULL;\
    if(v[c]) {\
      p##u##int##s##_t p = eval2p##u##int##s(v[c]);\
      if(p.p) {\
        i = (u##int##s##_t *)malloc(sizeof(u##int##s##_t));\
        if(!i) {\
          DM_ERR(ERR_SYSTEM, _("malloc failed\n"));\
          RETURN(ev);\
        }\
        *i = *p.p;\
      }\
    }\
    if(i) DM_DBG(DM_N(3), "evaluating[%u] to %"fp"\n", c, *i);\
    else DM_DBG(DM_N(3), "evaluating[%u] to NULL\n", c);\
    ev.push_back(i);\
  }\
\
  return ev;\
}
_EVAL_VEC_PINT(,32,"ld");
_EVAL_VEC_PINT(,64,"lld");
_EVAL_VEC_PINT(u,32,"lu");
_EVAL_VEC_PINT(u,64,"llu");
#undef _EVAL_VEC_PINT

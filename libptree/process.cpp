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

//#define USE_PTHREAD_ATTR              /* SEGVs! */

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
  
  DM_DBG(DM_N(3), "proc=%p, node=%p\n", p, p->n);
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", p->n->row, p->n->executed, p->n->evaluated, p->n);
  DM_DBG(DM_N(3), "%s\n", p->n->toString(FALSE).c_str());

  root_eval = p->eval_sequential_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, p->n->evaluated, root_eval);
  MUTEX(&thread.total_mtx, thread.total--);

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", p->n->row, p->n->executed, p->n->evaluated, pthread_self());
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
         p, p_request->tp_tid, p->n->executed, p->n->evaluated);
  
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
}

/*
 * Initialise Process
 */
void
Process::init(Node *node)
{
  n = node;
}

/*
 * Process constuctor
 */
Process::Process(Node *node)
{
  Process::init(node);
  init();
}

/*
 * Process destructor
 */
Process::~Process()
{
  for (uint u = 0; u < vProc.size(); u++) {
    tp_dequeue(vProc[u]);	/* dequeue any processes that didn't start yet */
    DELETE(vProc[u]);		/* terminate running? or finished processes */
  }
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
 */
int
Process::eval()
{
  DM_DBG_I;
  std::string str_node = Node::nodeToString(n, n->OFFSET, TRUE);
  DM_DBG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_LOG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", n->row, n->executed, n->evaluated, n);

  int root_eval, par_eval;
  int sreqs = 0;	/* number of parallel subrequests created */
  pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;

  /* Schedule parallel execution */
  if(n->par) {
    Node *ptr_node;
 
    DM_DBG(DM_N(3), FBRANCH"branches located at the same offset exist\n", n->row, n->executed, n->evaluated);

    /* check for parallel execution and create new requests if parallel branches found */
    for(ptr_node = n->par; ptr_node; ptr_node = ptr_node->par) {
      BOOL can_enqueue;
      can_enqueue = FALSE;
      
      if(ptr_node->COND == S2_COND_NONE) {
#if 1
        if(ptr_node->REPEAT.type == S2_REPEAT_PAR) {
          /* repeats execution */
          int repeats_eval;
          uint threads_total = 1 + ((ptr_node->REPEAT.X > ptr_node->REPEAT.Y)? ptr_node->REPEAT.X - ptr_node->REPEAT.Y: ptr_node->REPEAT.Y - ptr_node->REPEAT.X);

          DM_DBG(DM_N(1), _("parallel repeat "FBRANCH"total number of threads=%d\n"), ptr_node->row, ptr_node->executed, ptr_node->evaluated, threads_total);
          for(uint threads_i = 0; threads_i < threads_total; threads_i++) {
            S_P(&thread.total_mtx);
            if(thread.total < opts.tp_size) {
              thread.total++;
              can_enqueue = TRUE;
            }
            S_V(&thread.total_mtx);

            if(can_enqueue) { 
              DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", ptr_node->row, ptr_node->executed, ptr_node->evaluated);
              Process *proc = new Process(ptr_node);
              vProc.push_back(proc);
              if(tp_enqueue(proc, &sreqs, &sreqs_cv)) {
                DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), ptr_node->row, ptr_node->executed, ptr_node->evaluated, strerror(errno));
              } else {
                /* number of sub-requests */
                sreqs++;
                DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", ptr_node->row, ptr_node->executed, ptr_node->evaluated, sreqs);
              }
            } else {
              /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
                 => evaluate sequentially */
              DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", ptr_node->row, ptr_node->executed, ptr_node->evaluated, thread.total, opts.tp_size);
              repeats_eval = eval_with_timeout();
              UPDATE_MAX_MUTEX(&thread.evaluated_mtx, ptr_node->evaluated, repeats_eval);
            }
          }

          continue;
        }
#endif
        
        S_P(&thread.total_mtx);
        if(thread.total < opts.tp_size) {
          thread.total++;
          can_enqueue = TRUE;
        }
        S_V(&thread.total_mtx);
        
        if(can_enqueue) {
          Process *proc = new Process(ptr_node);
          vProc.push_back(proc);
          DM_DBG(DM_N(1), FBRANCH"enqueing a request\n", n->row, n->executed, n->evaluated);
          if(tp_enqueue(proc, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, n->executed, n->evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, n->executed, n->evaluated, sreqs);
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          Process proc = Process(ptr_node);
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, n->executed, n->evaluated, thread.total, opts.tp_size);
          root_eval = proc.eval_repeats();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, root_eval);
        }
      } /* if */
    } /* for */
  } else {
    DM_DBG(DM_N(3), FBRANCH"no branches located at the same offset exist\n", n->row, n->executed, n->evaluated);
  }

  root_eval = eval_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, root_eval);
  DM_DBG(DM_N(3), FBRANCH"root_eval=%d\n", n->row, n->executed, n->evaluated, root_eval);

  /* Investigate branches at the same offset */
  if(n->par) {
    Node *ptr_node = n->par;
    DM_DBG(DM_N(5), FBRANCH"investigating branches located at the same offset\n", n->row, n->executed, n->evaluated);
    switch(n->par->COND) {
      case S2_COND_OR:
        if(root_eval > n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"OR: root_eval=%d > EVAL=%d\n", n->row, n->executed, n->evaluated, root_eval, n->EVAL);
          Process proc = Process(n->par);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          n->evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", n->row, n->executed, n->evaluated, par_eval);
        } else {
          /* see if we have an AND node that we need to evaluate */
          DM_DBG(DM_N(5), FBRANCH"OR: evaluated(%d) <= EVAL(%d)\n", n->row, n->executed, n->evaluated, n->evaluated, n->EVAL);

          ptr_node = n->par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_AND) goto eval_and;
            ptr_node = ptr_node->par;
          }
          break;

eval_and:
          DM_DBG(DM_N(5), FBRANCH"found a par AND, evaluating branch %u\n", n->row, n->executed, n->evaluated, ptr_node->row);
          Process proc = Process(ptr_node);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          n->evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", n->row, n->executed, n->evaluated, par_eval);
        }
      break;

      case S2_COND_AND:
        if(root_eval <= n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) <= EVAL(%d)\n", n->row, n->executed, n->evaluated, root_eval, n->EVAL);
          Process proc = Process(n->par);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          n->evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, n->executed, n->evaluated, par_eval);
        } else {
          /* see if we have an OR node that can save us (return TRUE evaluation) */
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) > EVAL(%d)\n", n->row, n->executed, n->evaluated, root_eval, n->EVAL);

          ptr_node = n->par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_OR) goto eval_or;
            ptr_node = ptr_node->par;
          }
          break;

eval_or:
          DM_DBG(DM_N(5), FBRANCH"found a par OR, evaluating branch %u\n", n->row, n->executed, n->evaluated, ptr_node->row);
          Process proc = Process(ptr_node);
          par_eval = proc.eval();
          S_P(&thread.evaluated_mtx);
          n->evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, n->executed, n->evaluated, par_eval);
        }
      break;
      
      case S2_COND_NONE:
        /* parallel execution, taken care of by a new thread scheduled at the top */
        DM_DBG(DM_N(5), FBRANCH"found a parallel branch, execution already scheduled\n", n->row, n->evaluated, n->executed);
      break;
    }
  }

  S_P(&sreqs_mtx);
  while(sreqs != 0) {
    DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", n->row, n->executed, n->evaluated, sreqs);
    pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
  }
  S_V(&sreqs_mtx);

  /* threads-related cleanup */
  pthread_cond_destroy(&sreqs_cv);
  pthread_mutex_destroy(&sreqs_mtx);

  /* Parallel execution finished, we have one thread of execution. *
   * Go through the parallel branches and set the return value.    */
  if(n->par) {
    Node *ptr_node;
    for(ptr_node = n->par; ptr_node; ptr_node = ptr_node->par)
      if(ptr_node->COND == S2_COND_NONE)
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, ptr_node->evaluated);
  }

  DM_DBG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, n->executed, n->evaluated, root_eval);
  DM_LOG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, n->executed, n->evaluated, root_eval);

  RETURN(n->evaluated);
}

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Process::eval_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", n->row, n->executed, n->evaluated, this);

  int repeats_eval;

  switch(n->REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_OR:		/* fall through */
    case S2_REPEAT_AND:		/* fall through */
      repeats_eval = eval_sequential_repeats();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, repeats_eval);
      DM_DBG(DM_N(5), FBRANCH"repeats_eval=%d\n", n->row, n->executed, n->evaluated, repeats_eval);
    break;

    case S2_REPEAT_PAR:
      uint threads_total = 1 + ((n->REPEAT.X > n->REPEAT.Y)? n->REPEAT.X - n->REPEAT.Y: n->REPEAT.Y - n->REPEAT.X);
      int sreqs = 0;	/* number of parallel subrequests created */
      uint threads_i = 0;
      pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
      pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;
      DM_DBG(DM_N(1), _("parallel repeat "FBRANCH"total number of threads=%d\n"), n->row, n->executed, n->evaluated, threads_total);
      for(; threads_i < threads_total; threads_i++) {
        BOOL can_enqueue;
        can_enqueue = FALSE;
        
        S_P(&thread.total_mtx);
        if(thread.total < opts.tp_size) {
          thread.total++;
          can_enqueue = TRUE;
        }
        S_V(&thread.total_mtx);

        if(can_enqueue) { 
          DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", n->row, n->executed, n->evaluated);
          if(tp_enqueue(this, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, n->executed, n->evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, n->executed, n->evaluated, sreqs);
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, n->executed, n->evaluated, thread.total, opts.tp_size);
          repeats_eval = eval_with_timeout();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, repeats_eval);
        }
      }

      S_P(&sreqs_mtx);
      while(sreqs != 0) {
        DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", n->row, n->executed, n->evaluated, sreqs);
        pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
      }
      S_V(&sreqs_mtx);

      /* threads-related cleanup */
      pthread_cond_destroy(&sreqs_cv);
      pthread_mutex_destroy(&sreqs_mtx);

    break;
  }

  DM_DBG(DM_N(5), FBRANCH"eval_repeats returns\n", n->row, n->executed, n->evaluated);
  RETURN(n->evaluated);
} /* eval_repeats */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Process::eval_sequential_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", n->row, n->executed, n->evaluated, this);

  int8_t step = n->REPEAT.X < n->REPEAT.Y? 1 : -1;
  int iter_eval;
  n->REPEAT.I = n->REPEAT.X - step;

  switch(n->REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_PAR:		/* fall through: parallelism already handled in eval_repeats() */
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"\n", n->row, n->executed, n->evaluated);
      iter_eval = eval_with_timeout();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, iter_eval);
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"iter_eval=%d\n", n->row, n->executed, n->evaluated, iter_eval);
    break;

    case S2_REPEAT_OR:
      while(n->REPEAT.I != n->REPEAT.Y) {
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"REPEAT.I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", n->row, n->evaluated, n->executed, n->REPEAT.I, n->REPEAT.Y);
        n->REPEAT.I += step;

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, iter_eval);
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"iter_eval=%d\n", n->row, n->evaluated, n->executed, iter_eval);
        if(n->evaluated <= n->EVAL) {
          DM_DBG(DM_N(5), "OR repeat "FBRANCH"successfully evaluated; evaluated=%d <= EVAL=%d\n", n->row, n->executed, n->evaluated, n->evaluated, n->EVAL);
          /* end on first successful evaluation */
          break;
        }
      }
    break;

    case S2_REPEAT_AND:
      while(n->REPEAT.I != n->REPEAT.Y) {
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"REPEAT.I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", n->row, n->executed, n->evaluated, n->REPEAT.I, n->REPEAT.Y);
        n->REPEAT.I += step;

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, iter_eval);
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"iter_eval=%d\n", n->row, n->executed, n->evaluated, iter_eval);
        if(n->evaluated > n->EVAL) {
          DM_DBG(DM_N(5), "AND repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", n->row, n->executed, n->evaluated, n->evaluated, n->EVAL);
          /* end on first unsuccessful evaluation */
          break;
        }
      }
    break;

  }

  DM_DBG(DM_N(5), FBRANCH"eval_sequential_repeats returns\n", n->row, n->executed, n->evaluated);
  RETURN(n->evaluated);
} /* eval_sequential_repeats */

/*
 * function pthread_timeout_handler(): free thread-related data.
 * input:     pointer to the node.
 * output:    none
 */
static void
pthread_timeout_handler(void *node)
{
  DM_DBG_I;
  pthread_t tid = pthread_self();	/* thread identifying number */
  Node *n = (Node *)node;

  DM_DBG(DM_N(3), FBRANCH"cleaning up thread (%lu)\n", n->row, n->executed, n->evaluated, tid);

  S_V(&thread.total_mtx);
  S_V(&thread.evaluated_mtx);
  S_V(&thread.print_mtx);

  DM_DBG_O;
}

/*
 * For use in threads only.
 */
void *
eval_in_parallel_without_timeout(void *timeout_info)
{
  DM_DBG_I;

  timeout_info_t *ti = (timeout_info_t *)timeout_info;
  int root_eval;
  
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", ti->p->n->row, ti->p->n->executed, ti->p->n->evaluated, ti->p->n);
  DM_DBG(DM_N(3), "%s\n", ti->p->n->toString(FALSE).c_str());

  root_eval = ti->p->eval_without_timeout();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, ti->p->n->evaluated, root_eval);
  ti->terminated = TRUE;

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", ti->p->n->row, ti->p->n->executed, ti->p->n->evaluated, pthread_self());
  
  if(pthread_cond_broadcast(&thread.timeout_cv)) {
    DM_ERR(ERR_SYSTEM, _("pthread_cond_signal failed: %s\n"), strerror(errno));
  }

#if 0
  pthread_exit((void *)ti->p->n);
#else
  return (void *)ti->p->n;
#endif
} /* eval_in_parallel_notimeout */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Process::eval_with_timeout()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", n->row, n->executed, n->evaluated, this);

  pthread_t thread_id;
  struct timeval now;			/* timeval uses micro-seconds */
  struct timespec timeout;		/* timespec uses nano-seconds */
  timeout_info_t ti;

  /* disable timeouts until they're properly implemented and tested */
//  RETURN(eval_without_timeout());

  if(!n->TIMEOUT) {
    /* no timeout needed */
    DM_DBG(DM_N(3), FBRANCH"no timeout set\n", n->row, n->executed, n->evaluated);
    RETURN(eval_without_timeout());
  }

  /* set thread cleanup handler */
  DM_DBG(DM_N(3), FBRANCH"pushing cleanup handler\n", n->row, n->executed, n->evaluated);
  pthread_cleanup_push(pthread_timeout_handler, (void*)this);

  ti.p = this;
  ti.terminated = FALSE;
  
  /* timeouts handling */
  DM_DBG(DM_N(3), FBRANCH"node=%p; creating new thread with timeout=%"PRIu64"\n", n->row, n->executed, n->evaluated, this, n->TIMEOUT);
  int thread_rval = thread_create(&thread_id, eval_in_parallel_without_timeout, &ti);
  if(thread_rval)
  {
    DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create thread: %s\n"), n->row, n->executed, n->evaluated, strerror(errno));
  } else {
    /* thread was created fine */
    DM_DBG(DM_N(3), FBRANCH"thread %lu created successfully, adding timeout=%"PRIu64"\n", n->row, n->executed, n->evaluated, thread_id, n->TIMEOUT);
    
    if(gettimeofday(&now, NULL)) {
      DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
      RETURN(ERR_SYSTEM);
    }
    unsigned long timeout_add_sec = n->TIMEOUT / 1000000;
    unsigned long timeout_add_usec = n->TIMEOUT - ((n->TIMEOUT / 1000000) * 1000000);

    timeout.tv_sec = now.tv_sec + timeout_add_sec;
    timeout.tv_nsec = (now.tv_usec + timeout_add_usec) * 1000;

    S_P(&thread.timeout_mtx);
    DM_DBG_T(DM_N(2), FBRANCH"waiting for timeout_cv\n", n->row, n->executed, n->evaluated);
    int rc;
    while(1) {
      rc = pthread_cond_timedwait(&thread.timeout_cv, &thread.timeout_mtx, &timeout);
      if(rc == ETIMEDOUT) {
        /* timeout reached, cancel the thread */
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv timed out\n", n->row, n->executed, n->evaluated);
        break;
      }
      if(ti.terminated) {
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv triggered\n", n->row, n->executed, n->evaluated);
        break;
      }
    }
    S_V(&thread.timeout_mtx);

    if(rc == ETIMEDOUT) {
      /* timeout reached, cancel the thread */
      DM_DBG_T(DM_N(2), FBRANCH"cancelling thread %lu\n", n->row, n->executed, n->evaluated, thread_id);
      pthread_cancel(thread_id);
    }
  }

  /* ``reap'' the thread */
  Process *p;	/* pointer to a return value from a thread */
  DM_DBG(DM_N(3), "reaping thread %lu\n", n->row, n->executed, n->evaluated, thread_id);
  if(thread_join(thread_id, (void **)&p)) {
    DM_ERR(ERR_SYSTEM, FBRANCH"failed to join thread: %s\n", n->row, n->executed, n->evaluated, strerror(errno));
  }
  DM_DBG(DM_N(3), FBRANCH"joined thread; proc ptr=%p\n", n->row, n->executed, n->evaluated, p);
  if(p == PTHREAD_CANCELED) {
    /* timeout */
    int root_eval;
    DM_DBG(DM_N(2), FBRANCH"timeout reached\n", n->row, n->executed, n->evaluated);
    DM_LOG(DM_N(2), FBRANCH"timeout reached\n", n->row, n->executed, n->evaluated);
    root_eval = ERR_NEXEC;
    UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, ERR_NEXEC);
    if(opts.e1_fname) Node::print_node(n, n->OFFSET, opts.e1_file, TRUE, TRUE, FALSE);
  }

  /* Remove thread cleanup handler. */
  pthread_cleanup_pop(0);

  RETURN(n->evaluated);
} /* eval_with_timeout */

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_without_timeout()
{
  int no_timeout_exec, no_timeout_eval;

  if(opts.e0_fname) Node::print_node(n, n->OFFSET, opts.e0_file, TRUE, FALSE, FALSE);
  DM_DBG_T(DM_N(4), FBRANCH"starting execution of node=%p\n", n->row, n->executed, n->evaluated, this);
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(this, OFFSET, TRUE).c_str());
  n->has_executed = TRUE;		/* don't move this behind exec(), timeouts! */
  no_timeout_exec = n->exec();
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(this, OFFSET, TRUE).c_str());
  S_P(&thread.evaluated_mtx);
  n->executed = n->evaluated = no_timeout_exec;	/* for ${?} */
  S_V(&thread.evaluated_mtx);
  if(opts.e1_fname) Node::print_node(n, n->OFFSET, opts.e1_file, TRUE, TRUE, FALSE);
  DM_DBG_T(DM_N(4), FBRANCH"finished execution (of node=%p)\n", n->row, n->executed, n->evaluated, this);
  DM_LOG(DM_N(2), FBRANCH"executed=%d\n", n->row, n->executed, n->evaluated, no_timeout_exec);

  no_timeout_eval = eval_subtree(no_timeout_exec, no_timeout_exec);
  DM_DBG(DM_N(4), FBRANCH"node=%p; no_timeout_eval=%d\n", n->row, n->executed, n->evaluated, this, no_timeout_eval);

  S_P(&thread.evaluated_mtx);
  n->evaluated = no_timeout_eval;
  S_V(&thread.evaluated_mtx);

  return no_timeout_eval;
}

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_subtree(const int root_exec, int &root_eval)
{
  int child_eval; 

  /* investigate CHILDREN */
  if(root_exec <= n->EVAL) { 
    /* evaluate the children (only if root executed fine) */
    if(n->child) {
      DM_DBG(DM_N(3), FBRANCH"evaluating child branch %u\n", n->row, n->executed, n->evaluated, n->child->row);
      Process proc = Process(n->child);
      child_eval = proc.eval();
      root_eval = child_eval;
    }
  }

  DM_DBG(DM_N(5), FBRANCH"subtree_eval=%d\n", n->row, n->executed, n->evaluated, root_eval);

  return root_eval;
}
/* eval_subtree */

std::string
Process::toString(BOOL eval)
{
  std::stringstream ss;

  ss << "Process";

  return ss.str();
}

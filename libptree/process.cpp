#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"			/* Node */
#include "process.h"		/* Process */
#include "expr.h"		/* Expression evaluation */

#include "constants.h"
#include "i18.h"
#include "sysdep.h"		/* BOOL, STD_BUF, ... */

#include "free.h"		/* FREE(), DELETE() */
#include "opt.h"		/* OPT() */
#include "max.h"		/* UPDATE_MAX() */
#include "io.h"			/* file_ropen(), ... */
#include "s2.h"			/* opts (s2 options) */
#include "str.h"
#include "thread_pool.h"

/* Tags */
#include "printf.h"		/* $PRINTF{} */
#include "md5f.h"		/* md5 sum function */
#include "date.h"		/* $DATE{}, e.g.: week() */

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <stdlib.h>		/* exit(), system() */
#include <stdio.h>		/* stderr */
#include <errno.h>		/* errno */

#include <iostream>		/* std::string, cout, endl, ... */
#include <sstream>		/* ostringstream */
#include <fstream>              /* ifstream */

//#define USE_PTHREAD_ATTR	/* SEGVs! */

using namespace std;

/* simple macros */
#define UPDATE_MAX_MUTEX(mtx,m1,m2)\
  do {S_P(mtx); UPDATE_MAX(m1,m2); S_V(mtx);} while(0)

/* global variables */
struct tp_sync_t tp_sync;

typedef struct timeout_info_t {
  Process *p;
  BOOL terminated;
} timeout_info_t;

/* functions */
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
  UPDATE_MAX(p->evaluated, root_eval);
  MUTEX(&tp_sync.total_mtx, tp_sync.total--);

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
  fun = FALSE;
  FUN_OFFSET = 0;
  n = NULL;
  I = 0;
  et = EVAL_NONE;
  executed = ERR_OK;
  evaluated = ERR_OK;
  parent = NULL;
  rpar = NULL;
  var_tab = NULL;
}

/*
 * Initialise Process
 */
void
Process::init(Node *node, Process *p, Process *rpar)
{
  fun = FALSE;			/* not a function call by default */
  FUN_OFFSET = p? p->FUN_OFFSET: 0;
  n = node;

  parent = p;
  Process::rpar = rpar;
  I = 0;
  var_tab = NULL;		/* use paren't scope (by default) */

  /* create new table of variables if parallel process */
  if(n->REPEAT.type == S2_REPEAT_PAR || is_parallel()) {
    /* it is a parallel process, keep variables in the local variable space of this process */
    var_tab = new Vars_t();
    DM_DBG(DM_N(2), FBRANCH"created local variable table %p\n", n->row, executed, evaluated, var_tab);
  } else {
    /* Try to find the first rpar process which is also a parallel processes and *
     * use its variable scope (not a parallel repeat as they have several local  *
     * variable scopes.                                                          */
    Process *proc_ptr = rpar;
    while(proc_ptr) {
      if(proc_ptr->is_parallel()) {
        DM_DBG(DM_N(3), "branch %u is parallel\n", proc_ptr->n->row);
        var_tab = proc_ptr->var_tab;
        break;
      }
      proc_ptr = proc_ptr->rpar;
    }
  }
}

/*
 * Process constuctor
 */
Process::Process(Node *node, Process *p, Process *rpar)
{
  init();
  Process::init(node, p, rpar);
}

/*
 * Process destructor
 */
Process::~Process()
{
  /* delete finished processes spawned by this process */
//  DELETE_VEC(vProc);

  /* delete table of local variables */
  if(var_tab != &gl_var_tab)
     if(is_parallel() || (n && n->REPEAT.type == S2_REPEAT_PAR) ||
        fun) {
       /* be defensive: test for `n' is necessary as we * 
        * might be destroying an uninitialised process. */
       DELETE(var_tab);
     }
}

int
Process::threads_init(void)
{
  DM_DBG_I;

  memset(&tp_sync, 0, sizeof(tp_sync_t));

#if USE_PTHREAD_ATTR
  pthread_attr_init(&tp_sync.attr);
  pthread_attr_setstacksize(&tp_sync.attr, THREAD_STACK_SIZE);
  pthread_attr_setdetachstate(&tp_sync.attr, PTHREAD_CREATE_JOINABLE);
#endif

  pthread_mutex_init(&tp_sync.total_mtx, NULL);
  pthread_mutex_init(&tp_sync.print_mtx, NULL);
  pthread_cond_init(&tp_sync.timeout_cv, NULL);
  pthread_mutex_init(&tp_sync.timeout_mtx, NULL);

  RETURN(ERR_OK);
}

int
Process::threads_destroy(void)
{
#if USE_PTHREAD_ATTR
  pthread_attr_destroy(&tp_sync.attr);   /* valgrind complains */
#endif

  pthread_mutex_destroy(&tp_sync.total_mtx);
  pthread_mutex_destroy(&tp_sync.print_mtx);
  pthread_cond_destroy(&tp_sync.timeout_cv);
  pthread_mutex_destroy(&tp_sync.timeout_mtx);

  return ERR_OK;
}

/* 
 * Take care of parallel branches.
 */
int
Process::eval()
{
  DM_DBG_I;
  
  if(n == NULL) {
    /* be defensive */
    DM_ERR_ASSERT("n == NULL\n");
    RETURN(ERR_ASSERT);
  }
  
  DM_DBG(DM_N(3), FBRANCH"proc=%p\n", n->row, executed, evaluated, this);

  int root_eval;
  int sreqs = 0;	/* number of parallel subrequests created */
  pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;
  std::vector <Process *> vProc;	/* vector of parallel processes spawned by this process */

  /* Schedule parallel execution */
  if(n->par && n->COND == S2_COND_NONE) {
    /* S2_COND_NONE is important not to schedule parallel execution more than once *
     * e.g. when evaluating OR or AND branches                                     */
    Node *ptr_node;

    DM_DBG(DM_N(3), FBRANCH"branches located at the same offset exist\n", n->row, executed, evaluated);

    /* check for parallel execution and create new requests if parallel branches found */
    for(ptr_node = n->par; ptr_node; ptr_node = ptr_node->par) {
      BOOL can_enqueue;
      can_enqueue = FALSE;

      if(ptr_node->COND == S2_COND_NONE) {
        if(ptr_node->REPEAT.type == S2_REPEAT_PAR) {
          /* repeats execution */
          int repeats_eval;
          int8_t step = ptr_node->REPEAT.X < ptr_node->REPEAT.Y? 1 : -1;
          int64_t i = ptr_node->REPEAT.X - step;

          do {
            i += step;
            DM_DBG(DM_N(1), "parallel repeat branch %u; >%"PRIi64" %"PRIi64"; i=%"PRIi64"\n", ptr_node->row, ptr_node->REPEAT.X, ptr_node->REPEAT.Y, i);
            S_P(&tp_sync.total_mtx);
            if(tp_sync.total < opts.tp_size) {
              tp_sync.total++;
              can_enqueue = TRUE;
            }
            S_V(&tp_sync.total_mtx);

          if(can_enqueue) {
              Process *proc = new Process(ptr_node, parent, NULL);
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
              Process proc = Process(ptr_node, parent, NULL);
              proc.I = i;
              DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", ptr_node->row, tp_sync.total, proc.executed, proc.evaluated, opts.tp_size);
              repeats_eval = proc.eval_with_timeout();
              UPDATE_MAX(evaluated, repeats_eval);
            }
          } while(i != ptr_node->REPEAT.Y);

          continue;
        } /* end repeats */
        
        S_P(&tp_sync.total_mtx);
        if(tp_sync.total < opts.tp_size) {
          tp_sync.total++;
          can_enqueue = TRUE;
        }
        S_V(&tp_sync.total_mtx);
        
        if(can_enqueue) {
          Process *proc = new Process(ptr_node, parent, NULL);
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
          Process proc = Process(ptr_node, parent, NULL);
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, executed, evaluated, tp_sync.total, opts.tp_size);
          root_eval = proc.eval_repeats();
          UPDATE_MAX(evaluated, root_eval);
        }
      } /* if */
    } /* for */
  } else {
    DM_DBG(DM_N(3), FBRANCH"no branches located at the same offset exist\n", n->row, executed, evaluated);
  }

  root_eval = eval_repeats();
  UPDATE_MAX(evaluated, root_eval);
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
  std::vector<Process *>::const_iterator it;
  for (it = vProc.begin(); it != vProc.end(); it++) {
    UPDATE_MAX(evaluated, (*it)->evaluated);
    delete *it;
  }
  
  DM_DBG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);
  DM_LOG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);
  
  /* write e2 debugging/logging information */
  n->executed = executed; n->evaluated = evaluated;

  RETURN(evaluated);	/* ${!} */
}

/* 
 * This function specifically takes care of evaluating parallel repeats.
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
    case S2_REPEAT_WHILE:	/* fall through */
seq:
      I = n->REPEAT.X;		/* necessary for S2_REPEAT_NONE only */
      repeats_eval = eval_sequential_repeats();
      UPDATE_MAX(evaluated, repeats_eval);
      DM_DBG(DM_N(5), FBRANCH"repeats_eval=%d\n", n->row, executed, evaluated, repeats_eval);
    break;

    case S2_REPEAT_PAR: {
      int8_t step = n->REPEAT.X < n->REPEAT.Y? 1 : -1;
      int64_t i = n->REPEAT.X - step;
      uint threads_total = 1 + ((n->REPEAT.X > n->REPEAT.Y)? n->REPEAT.X - n->REPEAT.Y: n->REPEAT.Y - n->REPEAT.X);
      if(threads_total == 1)
        /* no parallel threads needed */
        goto seq;

      std::vector <Process *> vProc;	/* vector of parallel processes spawned by this process */
      int sreqs = 0;		/* number of parallel subrequests created */
      pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
      pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;
      DM_DBG(DM_N(1), FBRANCH"parallel repeat; total number of threads=%d\n", n->row, executed, evaluated, threads_total);
      do {
        i += step;
        DM_DBG(DM_N(1), FBRANCH"parallel repeat; >%"PRIi64" %"PRIi64"; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X, n->REPEAT.Y, i);
        BOOL can_enqueue;
        can_enqueue = FALSE;
        
        S_P(&tp_sync.total_mtx);
        if(tp_sync.total < opts.tp_size) {
          tp_sync.total++;
          can_enqueue = TRUE;
        }
        S_V(&tp_sync.total_mtx);

        if(can_enqueue) {
          DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", n->row, executed, evaluated);
          Process *proc = new Process(n, parent, NULL);
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
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, executed, evaluated, tp_sync.total, opts.tp_size);
          repeats_eval = eval_with_timeout();
          UPDATE_MAX(evaluated, repeats_eval);
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

      /* Parallel execution finished, we have one thread of execution. *
       * Go through the parallel branches and set the return value.    */
      std::vector<Process *>::const_iterator it;
      for (it = vProc.begin(); it != vProc.end(); it++) {
        UPDATE_MAX(evaluated, (*it)->evaluated);
        delete *it;
      }

      /* Investigate branches at the same offset */
      if(n->par)
        evaluated = eval_par();
        
    }
    break;
        
  }

  DM_DBG(DM_N(5), FBRANCH"eval_repeats returns\n", n->row, executed, evaluated);
  RETURN(evaluated);
} /* eval_repeats */

/* 
 * Evaluate branches of the tree joined at the same offset.
 */
int
Process::eval_par()
{
  DM_DBG_I;
  int par_eval;

  /* Investigate branches at the same offset */
  if(n->par) {
    Node *ptr_node = n->par;
    DM_DBG(DM_N(5), FBRANCH"investigating branches located at the same offset\n", n->row, executed, evaluated);
    switch(n->par->COND) {
      case S2_COND_OR:
        if(evaluated > n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"OR: evaluated=%d > EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          Process proc = Process(n->par, parent, this);
          par_eval = proc.eval();
          evaluated = par_eval;
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

eval_and: /* only used by the goto above */
          DM_DBG(DM_N(5), FBRANCH"found a par AND, evaluating branch %u\n", n->row, executed, evaluated, ptr_node->row);
          Process proc = Process(ptr_node, parent, this);
          par_eval = proc.eval();
          evaluated = par_eval;
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        }
      break;

      case S2_COND_AND:
        if(evaluated <= n->EVAL) {
          DM_DBG(DM_N(5), FBRANCH"AND: evaluated(%d) <= EVAL(%d)\n", n->row, executed, evaluated, evaluated, n->EVAL);
          Process proc = Process(n->par, parent, this);
          par_eval = proc.eval();
          evaluated = par_eval;
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        } else {
          /* see if we have an OR node that can save us (return TRUE evaluation) */
          DM_DBG(DM_N(5), FBRANCH"AND: evaluated(%d) > EVAL(%d)\n", n->row, executed, evaluated, evaluated, n->EVAL);

          ptr_node = n->par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_OR) goto eval_or;
            ptr_node = ptr_node->par;
          }
          break;

eval_or: /* only used by the goto above */
          DM_DBG(DM_N(5), FBRANCH"found a par OR, evaluating branch %u\n", n->row, executed, evaluated, ptr_node->row);
          Process proc = Process(ptr_node, parent, this);
          par_eval = proc.eval();
          evaluated = par_eval;
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", n->row, executed, evaluated, par_eval);
        }
      break;
      
      case S2_COND_NONE:
        /* parallel execution, already taken care of */
        DM_DBG(DM_N(5), FBRANCH"found a parallel branch, execution already scheduled\n", n->row, evaluated, executed);
      break;
    }
  }

  RETURN(evaluated);
}

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
    case S2_REPEAT_NONE: {	/* fall through */
    case S2_REPEAT_PAR:		/* fall through: parallelism already handled in eval_repeats() */
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"\n", n->row, executed, evaluated);
      iter_eval = eval_with_timeout();
      UPDATE_MAX(evaluated, iter_eval);
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);
    }
    break;

    case S2_REPEAT_OR: {
      I = i;
      do {
        I += step;
        DM_DBG(DM_N(5), FBRANCH"OR repeat; >%"PRIi64" %"PRIi64"; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X, n->REPEAT.Y, i);

        iter_eval = eval_with_timeout();
        UPDATE_MAX(evaluated, iter_eval);
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"iter_eval=%d\n", n->row, evaluated, executed, iter_eval);
        if(evaluated <= n->EVAL) {
          DM_DBG(DM_N(5), "OR repeat "FBRANCH"successfully evaluated; evaluated=%d <= EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first successful evaluation */
          break;
        }
      } while(I != n->REPEAT.Y);
    }
    break;

    case S2_REPEAT_AND: {
      I = i;
      do {
        I += step;
        DM_DBG(DM_N(5), FBRANCH"AND repeat; >%"PRIi64" %"PRIi64"; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X, n->REPEAT.Y, i);

        iter_eval = eval_with_timeout();
        UPDATE_MAX(evaluated, iter_eval);
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);
        if(evaluated > n->EVAL) {
          DM_DBG(DM_N(5), "AND repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first unsuccessful evaluation */
          break;
        }
      } while(I != n->REPEAT.Y);
    }
    break;

    case S2_REPEAT_WHILE: {
      do {
        DM_DBG(DM_N(5), "WHILE repeat "FBRANCH"\n", n->row, executed, evaluated);
        iter_eval = eval_with_timeout();
        UPDATE_MAX(evaluated, iter_eval);
        DM_DBG(DM_N(5), "WHILE repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);

      } while(evaluated <= n->EVAL);
      DM_DBG(DM_N(5), "WHILE repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
    }
    break;
  }

  DM_DBG(DM_N(5), FBRANCH"eval_sequential_repeats returns\n", n->row, executed, evaluated);

  /* Investigate branches at the same offset */
  if(n->par && n->REPEAT.type != S2_REPEAT_PAR)
    /* evaluation of parallel repeats already taken care of in eval_repeats() */
    evaluated = eval_par();

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

  S_V(&tp_sync.total_mtx);

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
  UPDATE_MAX(ti->p->evaluated, root_eval);
  ti->terminated = TRUE;

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", ti->p->n->row, ti->p->executed, ti->p->evaluated, pthread_self());
  
  if(pthread_cond_broadcast(&tp_sync.timeout_cv)) {
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
    unsigned long timeout_add_sec = n->TIMEOUT / 1000000;	/* TIMEOUT is in microseconds */
    unsigned long timeout_add_usec = n->TIMEOUT % 1000000;
    timeout_add_sec += (now.tv_usec + timeout_add_usec) / 1000000;
    DM_DBG_T(DM_N(2), FBRANCH"now.tv_sec=%ld; timeout_add_sec=%ld, timeout_add_usec=%ld\n", n->row, executed, evaluated, now.tv_sec, timeout_add_sec, timeout_add_usec);
    timeout.tv_sec = now.tv_sec + timeout_add_sec;
    timeout.tv_nsec = ((now.tv_usec + timeout_add_usec) % 1000) * 1000;

    DM_DBG_T(DM_N(2), FBRANCH"seting timeout to wait till sec=%ld; nsec=%ld [now: sec=%ld, usec=%ld]\n", n->row, executed, evaluated, timeout.tv_sec, timeout.tv_nsec, now.tv_sec, now.tv_usec);
    S_P(&tp_sync.timeout_mtx);
    DM_DBG_T(DM_N(2), FBRANCH"waiting for timeout_cv\n", n->row, executed, evaluated);
    int rc;
    while(1) {
      rc = pthread_cond_timedwait(&tp_sync.timeout_cv, &tp_sync.timeout_mtx, &timeout);
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
    S_V(&tp_sync.timeout_mtx);

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
    UPDATE_MAX(evaluated, ERR_NEXEC);
  }

  /* Remove thread cleanup handler. */
  pthread_cleanup_pop(0);

  RETURN(evaluated);
} /* exec_with_timeout */

/* 
 * Evaluate subtree of a node with a timeout.
 */
int
Process::eval_with_timeout()
{
  DM_DBG_I;
  int timeout_exec, subtree_eval;
  Process proc_fun;

  /* print the node with variables evaluated just before its execution */
  if(n->TYPE != N_DEFUN) {
    if(opts.e0_fname) Node::print_node(n, n->OFFSET - FUN_OFFSET, opts.e0_file, this, FALSE, FALSE);
  }

  DM_DBG_T(DM_N(4), FBRANCH"starting execution of proc=%p\n", n->row, executed, evaluated, this);

  et=EVAL_STATIC;
  DM_LOG_B(DM_N(1), "e0:%s\n", Node::nodeToString(n, n->OFFSET - FUN_OFFSET, this).c_str());
  et=EVAL_ALL;
  if(n->TYPE == N_FUN) {
    /* this is a function, we need to prepare function call process for it *
     * (this will never timeout)                                           */
    timeout_exec = ((nFun *)n)->exec(this, proc_fun);
  } else
  timeout_exec = exec_with_timeout();
  et=EVAL_STATIC;
  DM_LOG_B(DM_N(1), "e1:%s\n", Node::nodeToString(n, n->OFFSET - FUN_OFFSET, this).c_str());

  /* there might have been warnings (unset variables) during tag expansions */
  UPDATE_MAX(executed, timeout_exec);
  evaluated = executed;				/* for ${?} */

  /* print the node with variables evaluated just after its execution */
  if(n->TYPE != N_DEFUN) {
    if(opts.e1_fname) Node::print_node(n, n->OFFSET - FUN_OFFSET, opts.e1_file, this, TRUE, FALSE);
  }

  if(n->TYPE == N_FUN && timeout_exec == ERR_OK) {
    /* evaluate function definition if there were no errors during binding *
     * of the function (arguments/parameters mismatch, etc.)               */
    DM_DBG(DM_N(3), FBRANCH"Evaluating a function %p\n", n->row, executed, evaluated, proc_fun.n);
    std::string by_ref_vals = "";
    et=EVAL_ALL;
    if(proc_fun.n->child) {
      /* we have a function with non-empty body */
      Process proc_fun_body = Process(proc_fun.n->child, &proc_fun, NULL);
      int fun_eval = proc_fun_body.eval();
      ((nFun *)n)->exec_finish(this, proc_fun);
      UPDATE_MAX(executed, fun_eval);
      evaluated = executed;				/* for ${?} */

      by_ref_vals = ((nFun *)n)->getByRefVals(&proc_fun);
      if(by_ref_vals.length() > 0) by_ref_vals=" :" + by_ref_vals;
    }
    et=EVAL_STATIC;
    fprintf(opts.e0_file, "---> FUN %s%s\n", ((nFun *)n)->name->c_str(), by_ref_vals.c_str());
    fprintf(opts.e1_file, "%d:---> FUN %s%s\n", executed, ((nFun *)n)->name->c_str(), by_ref_vals.c_str());
  }

  DM_DBG_T(DM_N(4), FBRANCH"finished execution (of proc=%p)\n", n->row, executed, evaluated, this);
  DM_LOG(DM_N(2), FBRANCH"executed=%d\n", n->row, executed, evaluated, timeout_exec);
  if(n->TYPE == N_DEFUN && !fun) {
    /* we have a DEFUN node, don't evaluate its definition unless it is a function call */
    subtree_eval = ERR_OK;
  } else  {
    et=EVAL_ALL;
    subtree_eval = eval_subtree(timeout_exec, timeout_exec);
  }
  DM_DBG(DM_N(4), FBRANCH"proc=%p; subtree_eval=%d\n", n->row, executed, evaluated, this, subtree_eval);

  UPDATE_MAX(evaluated, subtree_eval);

  RETURN(evaluated);
} /* eval_with_timeout */

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_subtree(const int root_exec, int &root_eval)
{
  DM_DBG_I;
  int child_eval;

  /* investigate CHILDREN */
  if(root_exec <= n->EVAL) { 
    /* evaluate the children (only if root executed fine) */
    if(n->child) {
      DM_DBG(DM_N(3), FBRANCH"evaluating child branch %u\n", n->row, executed, evaluated, n->child->row);
      Process proc = Process(n->child, this, NULL);
      child_eval = proc.eval();
      root_eval = child_eval;
    }
  }

  DM_DBG(DM_N(5), FBRANCH"subtree_eval=%d\n", n->row, executed, evaluated, root_eval);

  RETURN(root_eval);
}
/* eval_subtree */

std::string
Process::toString()
{
  DM_DBG_I;
  std::stringstream ss;

  ss << "Process";

  RETURN(ss.str());
}

/*
 * Is this a root of a parallel process?
 * I.e. does it have a separate namespace for local vars?
 */
BOOL
Process::is_parallel()
{
  DM_DBG_I;
  
#define PAR_SEARCH(p)\
  node_ptr = n->p;\
  while(node_ptr) {\
    if(node_ptr->COND == S2_COND_NONE) {\
      DM_DBG(DM_N(3), "branch %u is parallel\n", n->row);\
      RETURN(TRUE);\
    }\
    node_ptr = node_ptr->p;\
  }

#if 0
  if(n->REPEAT.type == S2_REPEAT_PAR) {
    DM_DBG(DM_N(3), "branch %u is parallel\n", n->row);
    RETURN(TRUE);
  }
#endif

  if(!n || n->COND != S2_COND_NONE) 
     goto not_parallel;

  /* check if it has any branches connected to it in parallel */
  Node *node_ptr;

  DM_DBG(DM_N(3), "branch %u: n=%p; n->rpar=%p; n->par=%p\n", n->row, n, n->rpar, n->par);
  PAR_SEARCH(rpar);	/* up */
  PAR_SEARCH(par);	/* down */

not_parallel:
  if(n) DM_DBG(DM_N(3), "branch %u is NOT parallel%s\n", 
          n->row, (n->REPEAT.type == S2_REPEAT_PAR)? " (is parallel repeat though)": "");

  RETURN(FALSE);
#undef PAR_SEARCH
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
  match = pcre_matches(s_expected.c_str(), received, n->match_opt.pcre, this);
  
  return match;
} /* e_match */

BOOL
Process::e_match(const char *expected, const char *received)
{
  BOOL match;
  std::string s_expected = eval_str(expected, this);
  match = pcre_matches(s_expected.c_str(), received, n->match_opt.pcre, this);

  return match;
} /* e_match */

/*
 * Manipulation with variables.
 */
void
Process::WriteVariable(Vars_t *var_tab, const char *name, const char *value, int vlen)
{
  Vars_t::iterator iter;

  if(!var_tab) {
    DM_ERR_ASSERT(_("var_tab == NULL\n"));
    return;
  }

  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return;
  }

  if((iter = var_tab->find(name)) != var_tab->end()) {
    /* variable `name' exists, change its value */
    iter->second = (vlen < 0)? value : std::string(value, vlen);
  } else {
    /* we have a new variable */
    if(vlen < 0) var_tab->insert(std::pair<std::string, std::string>(name, value));
    else var_tab->insert(std::pair<std::string, std::string>(name, std::string(value, vlen)));
  }
  
  if(vlen < 0) DM_DBG(DM_N(0), _("wrote variable `%s' with value `%s' (to %p)\n"), name, value, var_tab);
  else DM_DBG(DM_N(0), _("wrote variable `%s' with value `%.*s' (to %p)\n"), name, vlen, value, var_tab);
} /* WriteVariable */

void
Process::WriteVariable(Process *proc, const char *name, const char *value, int vlen)
{
  if(!proc) {
    DM_ERR_ASSERT(_("proc == NULL\n"));
    return;
  }
  
  if(proc->var_tab == NULL) {
    /* we don't have a table of local variables */
    if(proc->parent == NULL) {
      /* we don't even have a parent (we've hit [single---var_tab == NULL] root) *
       * => use table of global variables                                        */
      WriteVariable(&gl_var_tab, name, value, vlen);
    } else {
      /* try parent's scope */
      WriteVariable(proc->parent, name, value, vlen);
    }
  } else {
    /* good, we have a table of local variables */
    WriteVariable(proc->var_tab, name, value, vlen);
  }
} /* WriteVariable */

void
Process::WriteVariable(const char *name, const char *value, int vlen)
{
  unsigned name_len, no_warn;
  
  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return;
  }
  
  name_len = strlen(name);
  no_warn = *name == '-';	/* shouldn't be necessary (not used, but be consistent with ReadVariable) */
  if(name_len > (no_warn + 2) && name[no_warn] == ':' && name[no_warn] == ':')
    /* ${::<name>} or ${-::<name>} */
    return WriteVariable(&gl_var_tab, name + no_warn + 2, value, vlen);

  WriteVariable(this, name + no_warn, value, vlen);
} /* WriteVariable */

void
Process::WriteVariable(const char *name, const char *value)
{
  WriteVariable(name, value, -1);
} /* WriteVariable */

const char *
Process::ReadVariable(Vars_t *var_tab, const char *name)
{
  Vars_t::iterator iter;

  if(!var_tab) {
    DM_ERR_ASSERT(_("var_tab == NULL\n"));
    return NULL;
  }

  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return NULL;
  }

  if((iter = var_tab->find(name)) != var_tab->end()) {
    /* variable exists */
    DM_DBG(DM_N(0), _("read variable `%s' with value `%s' (from %p)\n"), name, iter->second.c_str(), var_tab);
    return iter->second.c_str();
  } else {
    /* variable doesn't exist */
    DM_DBG(DM_N(0), _("failed to read variable `%s' from %p\n"), name, var_tab);
    return (const char *)NULL;
  }
} /* ReadVariable */

const char*
Process::ReadVariable(Process *proc, const char *name)
{
  if(!proc) {
    DM_ERR_ASSERT(_("proc == NULL\n"));
    return NULL;
  }

  if(proc->var_tab == NULL) {
    /* we don't have a table of local variables */
    if(proc->parent == NULL) {
      /* we don't even have a parent (we've hit [single---var_tab == NULL] root) *
       * => use table of global variables                                        */
      return ReadVariable(&gl_var_tab, name);
    } else {
      /* try parent's scope */
      return ReadVariable(proc->parent, name);
    }
  } else {
    /* good, we have a table of local variables for this process *
     * (it is a process running in parallel to others)           */
    const char *val = ReadVariable(proc->var_tab, name);
    if(val != NULL) return val;
      
    /* search for a local variable failed (not set in this scope), *
     * try local variable scope of the parent                      */
    if(proc->parent != NULL) return ReadVariable(proc->parent, name);
    else /* time to use global variable space (no parent process) */
      return ReadVariable(&gl_var_tab, name);
  }
} /* ReadVariable */

const char *
Process::ReadVariable(const char *name)
{
  unsigned name_len, no_warn;
  
  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return NULL;
  }
  
  name_len = strlen(name);
  no_warn = *name == '-';
  if(name_len > (no_warn + 2) && name[no_warn] == ':' && name[no_warn] == ':')
    /* ${::<name>} or ${-::<name>} */
    return ReadVariable(&gl_var_tab, name + no_warn + 2);

  return ReadVariable(this, name + no_warn);
} /* ReadVariable */

/*
 * Evaluate cstr and return evaluated std::string.
 */
std::string
Process::eval_str(const char *cstr, Process *proc)
{
#define CHK_EVAL_ALL\
  if(proc->et != EVAL_ALL) {\
    s.append("$" + std::string(state_name[state]) + "{" + target + "}");\
    state=sInit;\
    continue;\
  }
#define BALLANCED_OPL(str, s)\
  if(OPL(str)) {\
    /* we have a reference to a variable */\
    state = s;\
    tgt_chars = get_ballanced_br_param(target, opt + opt_off);\
    if(tgt_chars == 0)\
      /* no characters parsed, we hit \0 */\
      break;\
    i += tgt_chars + opt_off - 2;	/* compensate for +1 loop increment */\
    continue;\
  }

  DM_DBG_I;

  int i, c;
  int slen;
  std::string s;
  std::string target;
  enum s_eval { 
    sInit, sDollar, sVar, sEvar, sCounter, sExpr,  sRnd,  sDate,  sPrintf,
    sMd5,  sDefined,  sMatch,  sInt,
  } state = sInit;
  static const char* state_name[] = {
    "",    "",      "",   "ENV", "I",      "EXPR", "RND", "DATE", "PRINTF",
    "MD5", "DEFINED", "MATCH", "INT",
  };
  const char *opt;
  int opt_off;
  int bslash = 0;  	/* we had the '\\' character */

  if(cstr == NULL) {
    DM_ERR_ASSERT(_("c_str == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  if(!proc || proc->et == EVAL_NONE) return std::string(cstr);

  s.clear();
  slen = strlen(cstr);
  for(i = 0; i < slen; i++) {
//    DM_DBG(DM_N(0), "------>%d\n", i);
    c = cstr[i];
    opt = cstr + i;

    /* take care of escaped characters */
    if(bslash != 0) --bslash;
    if(c == '\\') {
      if(bslash) {
        /* two backslashes => no quoting */
        bslash = 0;
      } else {
        bslash = 2;
      }
    }

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
        int tgt_chars;
        BALLANCED_OPL("{", sVar) else
        BALLANCED_OPL("ENV{", sEvar) else
        BALLANCED_OPL("I{", sCounter) else
        BALLANCED_OPL("EXPR{", sExpr) else
        BALLANCED_OPL("RND{", sRnd) else
        BALLANCED_OPL("DATE{", sDate) else
        BALLANCED_OPL("PRINTF{", sPrintf) else
        BALLANCED_OPL("MD5{", sMd5) else
        BALLANCED_OPL("DEFINED{", sDefined) else
        BALLANCED_OPL("MATCH{", sMatch) else
        BALLANCED_OPL("INT{", sInt) else
        {
          /* return the borrowed dollar */
          s.push_back('$');
          state = sInit;
          break;
        }
      }
      continue;

      case sVar:{
        /* we have a complete variable */
        if(target == "?") {	/* ${?} */
          /* return parent's execution value */
          int parent_executed = proc->parent? proc->parent->executed: 0;
          DM_DBG(DM_N(4), "returning parent's execution value %d\n", parent_executed);
          s.append(i2str(parent_executed));
          state = sInit;
          continue;
        }
        if(target == "!") {	/* ${!} */
          /* return evaluation value of the branch at the same offset */
          int rpar_evaluated = proc->rpar? proc->rpar->evaluated: 0;
          DM_DBG(DM_N(4), "returning rpar evaluation value %d\n", rpar_evaluated);
          s.append(i2str(rpar_evaluated));
          state = sInit;
          continue;
        }
        /* classic variable ${var} */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: ${...${var}...} */

        const char *new_var;
        const char *read_var = target.c_str();
        unsigned no_warn = *read_var == '-';

        DM_DBG(DM_N(4), "read_var=|%s|\n", read_var);
        if((new_var = proc->ReadVariable(read_var)) != NULL) {
          DM_DBG(DM_N(4), "var %s=|%s|\n", target.c_str(), new_var);
          s.append(new_var);
        } else {
          /* the variable is not defined, output its name and increase the evaluation value */
          if(!no_warn && proc->et == EVAL_ALL) {
            UPDATE_MAX(proc->executed, ERR_WARN);
            DM_WARN(ERR_WARN, _("variable `%s' is unset\n"), target.c_str());
          }
          s.append("${" + std::string(read_var + no_warn) + "}");
        }
        state = sInit;
      }
      continue;

      case sEvar:{
        /* we have a complete environment variable */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $ENV{...${var}...} */

        const char *env_var;
        env_var = getenv(target.c_str());
        
        if(env_var) {
          DM_DBG(DM_N(4), "env var %s=|%s|\n", target.c_str(), env_var);
          s.append(env_var);
        } else {
          /* the environment variable is not defined, output its name */
          UPDATE_MAX(proc->executed, ERR_WARN);
          DM_WARN(ERR_WARN, _("environment variable `%s' is unset\n"), target.c_str());
          s.append("$ENV{" + target + "}");
        }
        state = sInit;
      }
      continue;

      case sCounter:{
        /* we have a reference to a repeat loop counter */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $I{...${var}...} */

        const char *word;
        word = target.c_str();
        char *endptr;
        int16_t i16;

        i16 = get_int16(word, &endptr, FALSE);
        if(endptr != word) {
          int64_t i64 = 0;
          Process *ptr_proc = proc;
          while(ptr_proc && i16 >= 0) {
            if(ptr_proc->n->REPEAT.type == S2_REPEAT_OR || 
               ptr_proc->n->REPEAT.type == S2_REPEAT_AND ||
               ptr_proc->n->REPEAT.type == S2_REPEAT_PAR)
            {
              DM_DBG(DM_N(4), "found a repeat branch; i16=%d\n", i16);
              if(!i16) {
                i64 = ptr_proc->I;
                DM_DBG(DM_N(4), "$I{%d}=%"PRIi64"\n", i16, i64);
                break;
              } 
              i16--;
            }
            ptr_proc = ptr_proc->parent;
          }
          if(ptr_proc == NULL) {
            UPDATE_MAX(proc->executed, ERR_WARN);
            DM_WARN(ERR_WARN, _(FBRANCH"couldn't find repeat loop for repeat counter $I{%s}!\n"), proc->n->row, proc->executed, proc->evaluated, word);
          }

          DM_DBG(DM_N(4), "writing repeat counter value: %"PRIi64"\n", i64);
          s.append(i2str(i64));
        } else {
          UPDATE_MAX(proc->executed, ERR_WARN);
          DM_WARN(ERR_WARN, _("cannot evaluate loop nesting `%s': %s\n"), word, _(strerror(errno)));
          s.append("$I{" + target + "}");
        }
        state = sInit;
      }
      continue;

      case sExpr:{
        /* we have a complete expression to evaluate */
        /* DO NOT evaluate target here, let Expr do the evaluation *
         * (e.g. strings with whitespace in them)                  */
        CHK_EVAL_ALL;
        DM_DBG(DM_N(4), "expr=|%s|\n", target.c_str());

        Expr e = Expr(target.c_str(), proc);
        s.append(e.parse().toString());
        state = sInit;
      }
      continue;

      case sRnd:{
        /* we have a maximum+1 random number */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $RND{...${var}...} */
        CHK_EVAL_ALL;
        DM_DBG(DM_N(4), "expr=|%s|\n", target.c_str());

        Expr e = Expr(target.c_str(), proc);
        Attr attr = e.parse();
        if(attr.type != INT) {
          DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s' to an integer\n"), proc->n->row, proc->executed, proc->evaluated, target.c_str());
        } else {
          /* srandom() is done elsewhere */
          s.append(i2str(random() % attr.v.i));
        }
        state = sInit;
      }
      continue;

      case sDate:{
        CHK_EVAL_ALL;

        struct timeval timestamp;
        struct tm *now;
        std::string date;
      
        gettimeofday(&timestamp, NULL);
        now = localtime(&(timestamp.tv_sec));

        if(target == "") {
          /* use the default format */
          date = ssprintf("%04d-%02d-%02d@%02d:%02d:%02d.%06lu",
                   now->tm_year+1900, now->tm_mon+1, now->tm_mday,
                   now->tm_hour, now->tm_min, now->tm_sec, timestamp.tv_usec);
        } else {
          int j, esc = 0, len = target.length();
          for(j = 0; j < len; j++) {
            if(!esc) {
              if(target[j] == '%') esc = 1;
              else s.push_back(target[j]);
              continue;
            }

            /* escape */
            esc = 0;
            switch(target[j]) {
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
                s.push_back(target[j]);
            }
          }
          if(esc) s.push_back('%');
        }
        s.append(date);
        state = sInit;
      }
      continue;

      case sPrintf:{
        char **argv = NULL;
        int spaces = 0;
        int j, plen;
        const char *target_cstr = target.c_str();

        plen = target.length();
        DM_DBG(DM_N(4), "target_cstr=|%s|\n", target_cstr);
        for(j = 0; j < plen; j++) {
          /* count the number of spaces to guess the maximum number of $PRINTF parameters (-2) */
          if(IS_WHITE(target_cstr[j])) spaces++;
        }
        DM_DBG(DM_N(4), "spaces+2=%d\n", spaces+2);
        if((argv = (char **)malloc(sizeof(char **) * (spaces + 2))) == (char **)NULL) {
          DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
          return s;
        }
        DM_DBG(DM_N(4), "argv=%p\n", argv);
        std::string arg;
        int l = 0;
        for(j = 0;; j++) {
          int chars;
          chars = get_dq_param(arg, target_cstr + l);
          if(chars == 0) break;
          arg = eval_str(arg.c_str(), proc);	/* evaluate things like: $PRINTF{...${var}...} */
          if((argv[j] = (char *)strdup(arg.c_str())) == (char *)NULL) {
            DM_ERR(ERR_SYSTEM, _("strdup failed\n"));
            return s;
          }
          l += chars;
        }
        argv[j--] = 0;

        s.append(ssprintf_chk(argv));
        for(;j >= 0; j--) {
          FREE(argv[j]);
        }
        FREE(argv);
        state = sInit;
      }
      continue;

      case sMd5:{
        /* we have a maximum+1 random number */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $MD5{...${var}...} */
        char md5str[33];
        gen_md5(md5str, target.c_str());
        s.append(md5str);
        state = sInit;
      }
      continue;

      case sDefined:{
        /* we have a variable to check whether it was defined */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $DEFINED{...${var}...} */
        s.append(proc->ReadVariable(target.c_str())? "1" : "0");
        state = sInit;
      }
      continue;

      case sMatch:{
        std::string expected;
        std::string received;
        int expected_len = get_dq_param(expected, target.c_str());
        get_dq_param(received, target.c_str() + expected_len);
        
        expected = eval_str(expected.c_str(), proc);	/* evaluate things like: $MATCH{...${var}...} */
        received = eval_str(received.c_str(), proc);	/* evaluate things like: $MATCH{...${var}...} */
        
        DM_DBG(DM_N(4), "matching `%s' and `%s'\n", expected.c_str(), received.c_str());
        s.append(pcre_matches(expected.c_str(),
                              received.c_str(),
                              proc->n->match_opt.pcre, proc)? "1": "0");

        state = sInit;
      }
      continue;

      case sInt:{
        /* DO NOT evaluate target here, let Expr do the evaluation *
         * (e.g. strings with whitespace in them)                  */
        CHK_EVAL_ALL;
        DM_DBG(DM_N(4), "expr=|%s|\n", target.c_str());

        Expr e = Expr(target.c_str(), proc);
        Attr a = e.parse();
        switch(a.type) {
          case INT:	/* we have an integer, good */
          break;
          
          case REAL:	/* round it up */
            a.type = INT;
            a.v.i = (int64_t)a.v.r;
          break;
          
          default:
            UPDATE_MAX(proc->executed, ERR_WARN);
            DM_WARN(ERR_WARN, _("couldn't evaluate expression `%s' to an integer\n"), target.c_str());
          break;
        }
        s.append(a.toString());
        state = sInit;
      }
      continue;

    }

    /* no TAGs found, simply copy the characters */
    s.push_back(c);

  }

  DM_DBG(DM_N(4), "state=%d\n", state);

  /* sanity checks */
  if(state == sDollar) {
    s.push_back('$');
  } else {
    if(state != sInit) {
      UPDATE_MAX(proc->executed, ERR_WARN);
      DM_WARN(ERR_WARN, _("unterminated $%s{\n"), state_name[state]);
      s.append("$" + std::string(state_name[state]) + "{" + target);
    }
  }
  
  RETURN(s);
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

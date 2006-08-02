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
#include "parse.h"		/* TFunctions, gl_var_tab */

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

/* global variables */
struct tp_sync_t tp_sync;

typedef struct timeout_info_t {
  Process *p;
  BOOL terminated;
  pthread_cond_t timeout_cv;
  pthread_mutex_t timeout_mtx;
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
  MUTEX(&tp_sync.total_mtx, p, tp_sync.total--);

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", p->n->row, p->executed, p->evaluated, pthread_self());

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
  ret = NULL;
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
  if((n->REPEAT.type == S2_REPEAT_PAR || is_parallel()) && n->TYPE != N_DEFUN) {
    /* it is a parallel process, keep variables in the local variable space of this process */
    var_tab = new Vars_t();
    DM_DBG(DM_N(2), FBRANCH"created local variable table %p\n", n->row, executed, evaluated, var_tab);
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
  /* delete table of local variables */
  if(var_tab != &gl_var_tab)
     if(is_parallel() || (n && n->REPEAT.type == S2_REPEAT_PAR) ||
        fun) {
       /* be defensive: test for `n' is necessary as we might    * 
        * be destroying an uninitialised process;                */
       DELETE(var_tab);
     }

  /* call a node-specific finish() method (e.g.: release process memory allocated by gSoap, ...) */
  if(n)
    /* yes, n can be NULL, see proc_fun in eval_with_timeout() */
    n->finish(this);
}


/*
 * Show a simple progress "bar"
 *
 * Params:
 *  -1: init
 *   0: hide
 *   1: show
 */
void
Process::progress(int show, Process *proc)
{
  static char state = '/';
  static BOOL hidden = TRUE;

  if(!opts.progress_bar)
    return;

  S_P(&tp_sync.print_mtx,proc);
  if(!show) {
    /* hide */
    if(!hidden) {
      /* on some systems (Windows) '\b' doesn't seem to delete */
      fprintf(stderr,"\b \b");
      fflush(stderr);
    }
 
    hidden = TRUE;
    S_V(&tp_sync.print_mtx);
    return;
  }
  
  if(show == -1)
    /* init */
    state = '/';

  if(show == -2)
    /* sleeping */
    state = 's';

  switch (state) {
    case '-': state = '\\'; break;
    case '\\': state = '|'; break;
    case '|': state = '/'; break;
    case '/': state = '-'; break;
    case 's': state = 's'; break;

    default: 
      state = '/';
    break;
  }
  if(!hidden) {
    /* on some systems (Windows) '\b' doesn't seem to delete */
    fprintf(stderr,"\b \b");
  }
    
  fputc(state, stderr);
  fflush(stderr);
  hidden = FALSE;
  S_V(&tp_sync.print_mtx);
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

  Process::progress(1,this);		/* show/change progress bar */
  
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
      BOOL enqueued;
      enqueued = FALSE;

      if(ptr_node->COND == S2_COND_NONE) {
        if(ptr_node->REPEAT.type == S2_REPEAT_PAR) {
          /* repeats execution */
          int repeats_eval;
          int64_t x = Expr::eval2i(ptr_node->REPEAT.X->c_str(), this);
          int64_t y = Expr::eval2i(ptr_node->REPEAT.Y->c_str(), this);
          int8_t step = x < y ? 1 : -1;
          int64_t i = x - step;

          do {
            i += step;
            DM_DBG(DM_N(1), "parallel repeat branch %u; >%s %s; i=%"PRIi64"\n", ptr_node->row, ptr_node->REPEAT.X->c_str(), ptr_node->REPEAT.Y->c_str(), i);
            S_P(&tp_sync.total_mtx,this);
            DM_DBG(DM_N(3), FBRANCH"<<< total_mtx\n", n->row, executed, evaluated);
            if(tp_sync.total < opts.tp_size) {
              tp_sync.total++;
              
              Process *proc = new Process(ptr_node, parent, NULL);
              proc->I = i;
              vProc.push_back(proc);
              DM_DBG(DM_N(1), "parallel repeat branch %u: enqueing a request\n", ptr_node->row);
              if(tp_enqueue(proc, &sreqs, &sreqs_mtx, &sreqs_cv)) {
                DM_ERR(ERR_SYSTEM, _("branch %u: failed to create new thread: %s\n"), ptr_node->row, strerror(errno));
              } else {
                /* number of sub-requests */
                enqueued = TRUE;
                sreqs++;
                DM_DBG(DM_N(3), "branch %u: subrequests=%d\n", ptr_node->row, sreqs);
              }
            }
            DM_DBG(DM_N(3), FBRANCH"total_mtx >>>\n", n->row, executed, evaluated);
            S_V(&tp_sync.total_mtx);
            
            if(!enqueued) {
              /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
                 => evaluate sequentially */
              Process proc = Process(ptr_node, parent, NULL);
              proc.I = i;
              DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", ptr_node->row, tp_sync.total, proc.executed, proc.evaluated, opts.tp_size);
              repeats_eval = proc.eval_with_timeout();
              UPDATE_MAX(evaluated, repeats_eval);
            }
          } while(i != y);

          continue;
        } /* end repeats */
        
        S_P(&tp_sync.total_mtx,this);
        DM_DBG(DM_N(3), FBRANCH"<<< total_mtx\n", n->row, executed, evaluated);
        if(tp_sync.total < opts.tp_size) {
          tp_sync.total++;
          Process *proc = new Process(ptr_node, parent, NULL);
          vProc.push_back(proc);
          DM_DBG(DM_N(1), FBRANCH"enqueing a request\n", n->row, executed, evaluated);
          if(tp_enqueue(proc, &sreqs, &sreqs_mtx, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            enqueued = TRUE;
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, executed, evaluated, sreqs);
          }
        }
        DM_DBG(DM_N(3), FBRANCH"total_mtx >>>\n", n->row, executed, evaluated);
        S_V(&tp_sync.total_mtx);
        
        if(!enqueued) {
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

  S_P(&sreqs_mtx,this);
  DM_DBG(DM_N(3), FBRANCH"<<< sreqs_mtx\n", n->row, executed, evaluated);
  while(sreqs != 0) {
    DM_DBG(DM_N(3), FBRANCH"waiting for sreqs=0 (%d)\n", n->row, executed, evaluated, sreqs);
    pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
  }
  DM_DBG(DM_N(3), FBRANCH"sreqs_mtx >>>\n", n->row, executed, evaluated);
  S_V(&sreqs_mtx);

  /* threads-related cleanup */
  pthread_cond_destroy(&sreqs_cv);
  pthread_mutex_destroy(&sreqs_mtx);

  /* Parallel execution finished, we have one thread of execution. *
   * Go through the parallel branches and set the return value.    */
  std::vector<Process *>::const_iterator iter;
  for (iter = vProc.begin(); iter != vProc.end(); iter++) {
    UPDATE_MAX(evaluated, (*iter)->evaluated);
    delete *iter;
  }
  
  DM_DBG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);
  DM_LOG(DM_N(2), FBRANCH"complete evaluation=%d\n", n->row, executed, evaluated, root_eval);
  
  /* write debugging/logging information for e2 */
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
      I = n->REPEAT.X? Expr::eval2i(n->REPEAT.X->c_str(), this) : 0;	/* necessary for S2_REPEAT_NONE only */
      repeats_eval = eval_sequential_repeats();
      UPDATE_MAX(evaluated, repeats_eval);
      DM_DBG(DM_N(5), FBRANCH"repeats_eval=%d\n", n->row, executed, evaluated, repeats_eval);
    break;

    case S2_REPEAT_PAR: {
      int64_t x = Expr::eval2i(n->REPEAT.X->c_str(), this);
      int64_t y = Expr::eval2i(n->REPEAT.Y->c_str(), this);
      int8_t step = x < y ? 1 : -1;
      int64_t i = x - step;
      uint threads_total = 1 + ((x > y)? x - y: y - x);
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
        DM_DBG(DM_N(1), FBRANCH"parallel repeat; >%s %s; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X->c_str(), n->REPEAT.Y->c_str(), i);
        BOOL enqueued;
        enqueued = FALSE;
        
        S_P(&tp_sync.total_mtx,this);
        DM_DBG(DM_N(3), FBRANCH"<<< total_mtx\n", n->row, executed, evaluated);

        if(tp_sync.total < opts.tp_size) {
          tp_sync.total++;

          /* enqueue the request */
          DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", n->row, executed, evaluated);
          Process *proc = new Process(n, parent, NULL);
          proc->I = i;
          vProc.push_back(proc);
          if(tp_enqueue(proc, &sreqs, &sreqs_mtx, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), n->row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            enqueued = TRUE;
            sreqs++;
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", n->row, executed, evaluated, sreqs);
          }
        }

        DM_DBG(DM_N(3), FBRANCH"total_mtx >>>\n", n->row, executed, evaluated);
        S_V(&tp_sync.total_mtx);

        if(!enqueued) {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          I = i;
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", n->row, executed, evaluated, tp_sync.total, opts.tp_size);
          repeats_eval = eval_with_timeout();
          UPDATE_MAX(evaluated, repeats_eval);
        }
      } while(i != y);

      S_P(&sreqs_mtx,this);
      DM_DBG(DM_N(3), FBRANCH"<<< r sreqs_mtx (%d)\n", n->row, executed, evaluated, sreqs);
      while(sreqs != 0) {
        DM_DBG(DM_N(3), FBRANCH"waiting for r sreqs=0 (%d)\n", n->row, executed, evaluated, sreqs);
        pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
      }
      DM_DBG(DM_N(3), FBRANCH"r sreqs_mtx (%d) >>>\n", n->row, executed, evaluated, sreqs);
      S_V(&sreqs_mtx);

      /* threads-related cleanup */
      pthread_cond_destroy(&sreqs_cv);
      pthread_mutex_destroy(&sreqs_mtx);

      /* Parallel execution finished, we have one thread of execution. *
       * Go through the parallel branches and set the return value.    */
      std::vector<Process *>::const_iterator iter;
      for (iter = vProc.begin(); iter != vProc.end(); iter++) {
        UPDATE_MAX(evaluated, (*iter)->evaluated);
        delete *iter;
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

  int64_t x = n->REPEAT.X? Expr::eval2i(n->REPEAT.X->c_str(), this) : 0;
  int64_t y = n->REPEAT.Y? Expr::eval2i(n->REPEAT.Y->c_str(), this) : 0;
  int8_t step = x < y ? 1 : -1;
  int64_t i = x - step;
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
        DM_DBG(DM_N(5), FBRANCH"OR repeat; >%s %s; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X->c_str(), n->REPEAT.Y->c_str(), i);

        iter_eval = eval_with_timeout();
        UPDATE_MAX(evaluated, iter_eval);
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"iter_eval=%d\n", n->row, evaluated, executed, iter_eval);
        if(evaluated <= n->EVAL) {
          DM_DBG(DM_N(5), "OR repeat "FBRANCH"successfully evaluated; evaluated=%d <= EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first successful evaluation */
          break;
        }
      } while(I != y);
    }
    break;

    case S2_REPEAT_AND: {
      I = i;
      do {
        I += step;
        DM_DBG(DM_N(5), FBRANCH"AND repeat; >%s %s; i=%"PRIi64"\n", n->row, executed, evaluated, n->REPEAT.X->c_str(), n->REPEAT.Y->c_str(), i);

        iter_eval = eval_with_timeout();
        UPDATE_MAX(evaluated, iter_eval);
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"iter_eval=%d\n", n->row, executed, evaluated, iter_eval);
        if(evaluated > n->EVAL) {
          DM_DBG(DM_N(5), "AND repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", n->row, executed, evaluated, evaluated, n->EVAL);
          /* end on first unsuccessful evaluation */
          break;
        }
      } while(I != y);
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

  Process *p = (Process *)proc;
  Mutexes_t::iterator iter;

  /* put it all in a diagnostics block => no warnings when compiling without libdiagnose */
  DM_BLOCK(DBG, DM_N(3),
    pthread_t tid = pthread_self();	/* thread identifying number */

    DM_DBG(DM_N(3), FBRANCH"cleaning up thread (%lu)\n", p->n->row, p->executed, p->evaluated, tid);
  );

  /* unlock all the locks locked by this process to be canceled */
  for(iter = mutex_tab.begin(); iter != mutex_tab.end(); iter++) {
    if(iter->second == p) S_V(&tp_sync.print_mtx);
  }

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
  
  /* set thread cleanup handler */
  DM_DBG(DM_N(3), FBRANCH"pushing cleanup handler for proc=%p\n", ti->p->n->row, ti->p->executed, ti->p->evaluated, ti->p->n);
  pthread_cleanup_push(pthread_timeout_handler, (void*)ti->p);

  DM_DBG(DM_N(3), "%s\n", ti->p->n->toString(FALSE).c_str());

  root_eval = ti->p->n->exec(ti->p);
  UPDATE_MAX(ti->p->evaluated, root_eval);
  ti->terminated = TRUE;

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", ti->p->n->row, ti->p->executed, ti->p->evaluated, pthread_self());

  DM_DBG(DM_N(3), "<<< ti->timeout_mtx\n");
  S_P(&ti->timeout_mtx,ti->p);
  if(pthread_cond_broadcast(&ti->timeout_cv)) {
    DM_ERR(ERR_SYSTEM, _("pthread_cond_signal failed: %s\n"), strerror(errno));
  }
  S_V(&ti->timeout_mtx);
  DM_DBG(DM_N(3), "ti->timeout_mtx >>>\n");

#if 0
  pthread_exit((void *)ti->p);
#else
  return (void *)ti->p;
#endif

  /* Remove thread cleanup handler. */
  pthread_cleanup_pop(0);
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
  pthread_mutex_init(&ti.timeout_mtx, NULL);
  pthread_cond_init(&ti.timeout_cv, NULL);

  if(!n->TIMEOUT) {
    /* no timeout needed */
    DM_DBG(DM_N(3), FBRANCH"no timeout set\n", n->row, executed, evaluated);
    RETURN(n->exec(this));
  }

  ti.p = this;
  ti.terminated = FALSE;
  
  /* timeouts handling */
  DM_DBG(DM_N(3), FBRANCH"proc=%p; creating new thread with timeout=%"PRIu64"\n", n->row, executed, evaluated, this, n->TIMEOUT);
  S_P(&ti.timeout_mtx,this);
  DM_DBG(DM_N(3), FBRANCH"<<< timeout_mtx\n", n->row, executed, evaluated);
  int thread_rval = thread_create(&thread_id, exec_in_parallel_without_timeout, &ti);
  if(thread_rval)
  {
    DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create thread; evaluating without timeout: %s\n"), n->row, executed, evaluated, strerror(errno));

    /* cleanup */
    S_V(&ti.timeout_mtx);
    pthread_cond_destroy(&ti.timeout_cv);
    pthread_mutex_destroy(&ti.timeout_mtx);

    RETURN(n->exec(this));
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

    DM_DBG_T(DM_N(2), FBRANCH"setting timeout to wait till sec=%ld; nsec=%ld [now: sec=%ld, usec=%ld]\n", n->row, executed, evaluated, timeout.tv_sec, timeout.tv_nsec, now.tv_sec, now.tv_usec);
    int rc;
    while(1) {
      DM_DBG(DM_N(3), FBRANCH"pthread_cond_timedwait()\n", n->row, executed, evaluated);
      rc = pthread_cond_timedwait(&ti.timeout_cv, &ti.timeout_mtx, &timeout);
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
    DM_DBG(DM_N(3), FBRANCH"timeout_mtx >>>\n", n->row, executed, evaluated);
    S_V(&ti.timeout_mtx);

    if(rc == ETIMEDOUT) {
      /* timeout reached, cancel the thread */
      DM_DBG_T(DM_N(2), FBRANCH"cancelling thread %lu\n", n->row, executed, evaluated, thread_id);
      pthread_cancel(thread_id);
    }
  }

  /* ``reap'' the thread */
  Process *p;	/* pointer to a return value from a thread */
  DM_DBG(DM_N(3), FBRANCH"reaping thread %lu\n", n->row, executed, evaluated, thread_id);
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

  /* threads-related cleanup */
  pthread_cond_destroy(&ti.timeout_cv);
  pthread_mutex_destroy(&ti.timeout_mtx);

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
  et=EVAL_STATIC;
  if(n->TYPE != N_DEFUN) {
    if(opts.e0_fname) Node::print_node(n, n->OFFSET - FUN_OFFSET, opts.e0_file, this, FALSE, FALSE);
  }

  DM_DBG_T(DM_N(4), FBRANCH"starting execution of proc=%p\n", n->row, executed, evaluated, this);

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

  /* there might have been warnings (e.g. unset variables) during tag expansions */
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
    if(opts.e0_fname) fprintf(opts.e0_file, "---> FUN %s%s\n", ((nFun *)n)->name->c_str(), by_ref_vals.c_str());
    if(opts.e1_fname) fprintf(opts.e1_file, "%d:---> FUN %s%s\n", executed, ((nFun *)n)->name->c_str(), by_ref_vals.c_str());
  }

  DM_DBG_T(DM_N(4), FBRANCH"finished execution (of proc=%p)\n", n->row, executed, evaluated, this);
  DM_LOG(DM_N(2), FBRANCH"executed=%d\n", n->row, executed, evaluated, timeout_exec);
  if(n->TYPE == N_DEFUN && !fun) {
    /* we have a DEFUN node, don't evaluate its definition unless it is a function call */
    subtree_eval = ERR_OK;
  } else  {
    et=EVAL_ALL;
    subtree_eval = eval_subtree(executed);
  }
  DM_DBG(DM_N(4), FBRANCH"proc=%p; subtree_eval=%d\n", n->row, executed, evaluated, this, subtree_eval);

  UPDATE_MAX(evaluated, subtree_eval);

  RETURN(evaluated);
} /* eval_with_timeout */

/* 
 * Evaluate subtree of a node.
 */
int
Process::eval_subtree(const int root_exec)
{
  DM_DBG_I;
  int child_eval = ERR_OK;

  /* investigate CHILDREN */
  if(root_exec <= n->EVAL) { 
    /* evaluate the children (only if root executed fine) */
    if(n->child) {
      DM_DBG(DM_N(3), FBRANCH"evaluating child branch %u\n", n->row, executed, evaluated, n->child->row);
      Process proc = Process(n->child, this, NULL);
      child_eval = proc.eval();
    }
  }

  DM_DBG(DM_N(5), FBRANCH"subtree_eval=%d\n", n->row, executed, evaluated, child_eval);

  RETURN(child_eval);
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

inline static Process *
get_parent_scope(char **name, Process *up)
{
  char *endptr;
  const char *word;
  int16_t i16;

  if(name == NULL) {
    DM_ERR_ASSERT("name == NULL\n");
    RETURN(NULL);
  }

  word = *name;
  i16 = get_int16(word, &endptr, FALSE);
  if(endptr == word) {
    DM_ERR_ASSERT("endptr == word\n");
    RETURN(NULL);
  }
  *name = endptr;

  DM_DBG(DM_N(3), "nesting=%d\n", i16);
  
  while(i16--) {
    while(up && !up->var_tab) {
      up = up->parent;
    }
    if(up) up = up->parent;	/* skip */
    else break;
  }

  DM_DBG(DM_N(3), "up=%p\n", up);
  return up;
}

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
  
  if(vlen < 0) DM_DBG(DM_N(2), _("wrote variable `%s' with value `%s' (to %p)\n"), name, value, var_tab);
  else DM_DBG(DM_N(2), _("wrote variable `%s' with value `%.*s' (to %p)\n"), name, vlen, value, var_tab);
} /* WriteVariable */

void
Process::WriteVariable(Process *proc, const char *name, const char *value, int vlen)
{
  if(!proc) {
    DM_DBG(DM_N(4), _("proc == NULL\n"));
    return WriteVariable(&gl_var_tab, name, value, vlen);
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
Process::WriteVariable(const char *name, const char *value, BOOL argv, int vlen)
{
  unsigned name_len, no_warn;
  
  if(!name) {
    DM_ERR_ASSERT(_("name == NULL\n"));
    return;
  }
  
  name_len = strlen(name);
  no_warn = *name == '-';	/* shouldn't be necessary (not used, but be consistent with ReadVariable) */

  DM_DBG(DM_N(3), _("process=%p, process->var_tab=%p; parent=%p; parent->var_tab=%p\n"), this, this->var_tab, parent, parent? parent->var_tab : NULL);

  if(name_len >= (no_warn + 1)) {
    /* see ReadVariable amd note the difference in condition (> vs. >=) */
    if(name[no_warn] == '0') {
      /* ${0<name>} or ${-0<name>} */
      if(name[no_warn + 1] || argv) {
        WriteVariable(&gl_var_tab, name + no_warn + (argv? 0 : 1), value, vlen);
      } else {
        UPDATE_MAX(executed, ERR_WARN);
        DM_WARN(ERR_WARN, _("refusing to write read-only variable ${0}\n"));
      }
      return;
    }

    if(IS_ASCII_DIGIT(name[no_warn])) {
      /* ${[1-9]+<name>} or ${-[1-9]+<name>} */
      char *true_name = (char *)(name + no_warn);
      if(argv) {
        WriteVariable(&gl_var_tab, name + no_warn, value, vlen);
      } else if(*true_name) {
        Process *up = get_parent_scope(&true_name, this);
        DM_DBG(DM_N(4), _("this->var_tab=%p; parent->var_tab=%p; up->var_tab=%p\n"), this->var_tab, parent? parent->var_tab : NULL, up? up->var_tab : NULL);
        WriteVariable(up, true_name, value, vlen);
      } else {
        DM_WARN(ERR_WARN, _("refusing to write read-only variable ${%s}\n"), name + no_warn);
      }
      return;
    }
  }

  WriteVariable(this, name + no_warn, value, vlen);
} /* WriteVariable */

void
Process::WriteVariable(const char *name, const char *value, BOOL argv)
{
  WriteVariable(name, value, argv, -1);
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
    DM_DBG(DM_N(2), _("read variable `%s' with value `%s' (from %p)\n"), name, iter->second.c_str(), var_tab);
    return iter->second.c_str();
  } else {
    /* variable doesn't exist */
    DM_DBG(DM_N(2), _("failed to read variable `%s' from %p\n"), name, var_tab);
    return (const char *)NULL;
  }
} /* ReadVariable */

const char*
Process::ReadVariable(Process *proc, const char *name)
{
  if(!proc) {
    DM_DBG(DM_N(4), _("proc == NULL\n"));
    return ReadVariable(&gl_var_tab, name);
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

  DM_DBG(DM_N(3), _("process=%p, process->var_tab=%p; parent=%p; parent->var_tab=%p\n"), this, this->var_tab, parent, parent? parent->var_tab : NULL);
  
  if(name_len > (no_warn + 1)) {
    if(name[no_warn] == '0')
      /* ${0<name>} or ${-0<name>} */
      return ReadVariable(&gl_var_tab, name + no_warn + 1);

    if(IS_ASCII_DIGIT(name[no_warn])) {
      /* ${[1-9]+<name>} or ${-[1-9]+<name>} */
      char *true_name = (char *)(name + no_warn);
      Process *up = get_parent_scope(&true_name, this);
      DM_DBG(DM_N(4), _("this->var_tab=%p; parent->var_tab=%p; up->var_tab=%p\n"), this->var_tab, parent? parent->var_tab : NULL, up? up->var_tab : NULL);
      return ReadVariable(up, true_name);
    }
  }
  
  return ReadVariable(this, name + no_warn);
} /* ReadVariable */

/*
 * Evaluate the $SPLIT{} tags.
 */
std::string
Process::preeval_str(const char *cstr)
{
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
    sInit, sDollar, sSplit,
  } state = sInit;
  static const char* state_name[] = {
    "",    "",     "SPLIT",
  };
  const char *opt;
  int opt_off;
  int bslash = 0;  	/* we had the '\\' character */

  if(cstr == NULL) {
    DM_ERR_ASSERT(_("c_str == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  if(et == EVAL_NONE) return std::string(cstr);

  s.clear();
  slen = strlen(cstr);
  for(i = 0; i < slen; i++) {
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
        BALLANCED_OPL("SPLIT{", sSplit) else
        {
          /* return the borrowed dollar */
          s.push_back('$');
          state = sInit;
          break;
        }
      }
      continue;

      case sSplit:{
        target = eval_str(target.c_str(), this);	/* evaluate => expand variables, */
        if(et == EVAL_ALL) {
          s.append(target);
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
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
      UPDATE_MAX(executed, ERR_WARN);
      DM_WARN(ERR_WARN, _("unterminated $%s{\n"), state_name[state]);
      s.append("$" + std::string(state_name[state]) + "{" + target);
    }
  }

  RETURN(s);

#undef BALLANCED_OPL
} /* preeval_str */

std::string
Process::preeval_str(const std::string *s)
{
  if(s == NULL) {
    DM_ERR_ASSERT(_("s == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  return preeval_str(s->c_str());
} /* preeval_str */

/*
 * Evaluate cstr and return evaluated std::string.
 */
std::string
Process::eval_str(const char *cstr, Process *proc)
{
#define BALLANCED_OPL(str, s)\
  if(OPL(str)) {\
    /* we have a reference to a variable */\
    state = s;\
    tgt_chars = get_ballanced_br_param(target, opt + opt_off);\
    if(tgt_chars == 0) {\
      /* no characters parsed, we hit \0 */\
      UPDATE_MAX(proc->executed, ERR_WARN);\
      DM_WARN(ERR_WARN, _("unterminated $%s{\n"), state_name[state]);\
    }\
    DM_DBG(DM_N(6), "tgt_chars=%d\n", tgt_chars);\
    i += tgt_chars + opt_off - 2;	/* compensate for +1 loop increment */\
    target = proc->preeval_str(target.c_str());\
    continue;\
  }

  DM_DBG_I;

  int i, c;
  int slen;
  std::string s;
  std::string target;
  enum s_eval { 
    sInit, sDollar, sVar, sEvar, sCounter, sEval,  sExpr,  sRnd,  sDate,  sPrintf,
    sMd5,  sDefined,  sMatch,  sInt,  sFun,  sMap,  sInterleave,  sSeq,
  } state = sInit;
  static const char* state_name[] = {
    "",    "",      "",   "ENV", "I",      "EVAL", "EXPR", "RND", "DATE", "PRINTF",
    "MD5", "DEFINED", "MATCH", "INT", "FUN", "MAP", "INTERLEAVE", "SEQ",
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
        BALLANCED_OPL("EVAL{", sEval) else
        BALLANCED_OPL("RND{", sRnd) else
        BALLANCED_OPL("DATE{", sDate) else
        BALLANCED_OPL("PRINTF{", sPrintf) else
        BALLANCED_OPL("MD5{", sMd5) else
        BALLANCED_OPL("DEFINED{", sDefined) else
        BALLANCED_OPL("MATCH{", sMatch) else
        BALLANCED_OPL("INT{", sInt) else
        BALLANCED_OPL("FUN{", sFun) else
        BALLANCED_OPL("MAP{", sMap) else
        BALLANCED_OPL("INTERLEAVE{", sInterleave) else
        BALLANCED_OPL("SEQ{", sSeq) else
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

        DM_DBG(DM_N(4), "target=|%s|\n", target.c_str());
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

      case sEval:{
        target = eval_str(target.c_str(), proc);
        target = eval_str(target.c_str(), proc);
        DM_DBG(DM_N(4), "$eval=|%s|\n", target.c_str());

        if(proc->et == EVAL_ALL) {
          s.append(target.c_str());
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
        state = sInit;
      }
      continue;

      case sExpr:{
        /* we have a complete expression to evaluate */
        /* DO NOT evaluate target here, let Expr do the evaluation *
         * (e.g. strings with whitespace in them)                  */
        DM_DBG(DM_N(4), "expr=|%s|\n", target.c_str());

        if(proc->et == EVAL_ALL) {
          s.append(Expr::eval2s(target.c_str(), proc));
        } else {
          target = eval_str(target.c_str(), proc);	/* evaluate things like: $EXPR{...${var}...} */
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
        state = sInit;
      }
      continue;

      case sRnd:{
        /* we have a maximum+1 random number */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $RND{...${var}...} */
        DM_DBG(DM_N(4), "rnd=|%s|\n", target.c_str());

        if(proc->et == EVAL_ALL) {
          /* srandom() is done elsewhere */
          int64_t max = (int64_t)Expr::eval2i(target.c_str(), proc);
          if(max) s.append(i2str(random() % max));
          else  s.append("0");
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
        state = sInit;
      }
      continue;

      case sDate:{
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $DATE{...${var}...} */

        if(proc->et == EVAL_ALL) {
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
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
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
          state = sInit;
          continue;
        }
        DM_DBG(DM_N(4), "argv=%p\n", argv);
        std::string arg;
        int l = 0;
        for(j = 0;; j++) {
          BOOL ws_only;
          int chars;
          chars = get_dq_param(arg, target_cstr + l, ws_only);
          if(ws_only) break;
          arg = eval_str(arg.c_str(), proc);	/* evaluate things like: $PRINTF{...${var}...} */
          DM_DBG(DM_N(4), "arg=|%s|\n", arg.c_str());
          if((argv[j] = (char *)strdup(arg.c_str())) == (char *)NULL) {
            DM_ERR(ERR_SYSTEM, _("strdup failed\n"));
            state = sInit;
            continue;
          }
          l += chars;
        }
        argv[j--] = 0;

        DM_DBG(DM_N(4), "s=|%s|\n", s.c_str());
        if(proc->et == EVAL_ALL) {
          if(j >= 0) /* no SEGVs when $PRINTF{} (empty argument) */
            s.append(ssprintf_chk(argv));
          for(;j >= 0; j--) FREE(argv[j]);
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          for(int x = 0; x <= j; x++) {
            if(x != 0) s.push_back(' ');
            s.append(dq_param(argv[x], TRUE));
            FREE(argv[x]);
          }
          s.append("}");
        }
        FREE(argv);
        DM_DBG(DM_N(4), "s=|%s|\n", s.c_str());
        state = sInit;
      }
      continue;

      case sMd5:{
        /* we have a maximum+1 random number */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $MD5{...${var}...} */
        if(proc->et == EVAL_ALL) {
          char md5str[33];
          gen_md5(md5str, target.c_str());
          s.append(md5str);
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
        state = sInit;
      }
      continue;

      case sDefined:{
        /* we have a variable to check whether it was defined */
        target = eval_str(target.c_str(), proc);	/* evaluate things like: $DEFINED{...${var}...} */
        if(proc->et == EVAL_ALL) {
          s.append(proc->ReadVariable(target.c_str())? "1" : "0");
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
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

        if(proc->et == EVAL_ALL) {
          DM_DBG(DM_N(4), "matching `%s' and `%s'\n", expected.c_str(), received.c_str());
          s.append(pcre_matches(expected.c_str(),
                                received.c_str(),
                                proc->n->match_opt.pcre, proc)? "1": "0");
        } else {
          /* do not evaluate, show what we're matching only */
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(dq_param(expected.c_str(), TRUE) + " ");
          s.append(dq_param(received.c_str(), TRUE));
          s.append("}");
        }

        state = sInit;
      }
      continue;

      case sInt:{
        if(proc->et == EVAL_ALL) {
          /* DO NOT evaluate target here, let Expr do the evaluation *
           * (e.g. strings with whitespace in them)                  */
          s.append(i2str((int64_t)Expr::eval2r(target.c_str(), proc)));
        } else {
          target = eval_str(target.c_str(), proc);	/* evaluate things like: $INT{...${var}...} */
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(target.c_str());
          s.append("}");
        }
        state = sInit;
      }
      continue;

      case sFun:{
        TFunctions::iterator iter;		/* name/function object pair */
        const char *target_cstr = target.c_str();
        nDefun *nDefunNode = NULL;
        Process proc_fun;
        int chars;
        std::string name;
        std::vector <std::string *> args;	/* vector of call by value arguments */

        chars = get_dq_param(name, target_cstr);/* function name */
        name = eval_str(name.c_str(), proc);	/* evaluate things like: $FUN{...${var}... <arguments>} */

        DM_DBG(DM_N(4), "target=|%s|\n", target.c_str());
        DM_DBG(DM_N(4), "function name=|%s|\n", name.c_str());
        if(!nDefunNode) {
          if((iter = gl_fun_tab.find(name.c_str())) == gl_fun_tab.end()) {
            /* Function `name' is not defined. */
            UPDATE_MAX(proc->executed, ERR_ERR);
            DM_ERR(ERR_ERR, _("function `%s' not defined\n"), name.c_str());
            state = sInit;
            continue;
          }
        
          nDefunNode = iter->second;
        }
      
        /* get the number of parameters */
        uint params_size = nDefunNode->params.size();

        int l = chars;
        while(1) {
          BOOL ws_only;
          std::string arg;
          chars = get_dq_param(arg, target_cstr + l, ws_only);
          arg = eval_str(arg.c_str(), proc);	/* evaluate things like: $FUN{name ...${var}...} */
          DM_DBG(DM_N(4), "arg=|%s|\n", arg.c_str());
          if(ws_only) break;
          std::string *new_arg = new std::string(arg);
          if(new_arg == (std::string *)NULL) {
            DM_ERR(ERR_SYSTEM, _("new failed\n"));
            state = sInit;
            continue;
          }
          args.push_back(new_arg);
          l += chars;
        }

        uint args_size = args.size();
        
        if(proc->et == EVAL_ALL) {
          DM_DBG(DM_N(4), "args_size=%d; params_size=%d\n", args_size, params_size);
          if(args_size != params_size)
          {
            UPDATE_MAX(proc->executed, ERR_ERR);
            DM_ERR(ERR_ERR, _("number of function `%s' parameters (%u) != number of its arguments (%u) passed by value\n"), name.c_str(), params_size, args_size);
            state = sInit;
            continue;
          }
          
          DM_DBG(DM_N(4), "creating new process\n");
          proc_fun = Process(nDefunNode, proc, NULL);
//          proc_fun.fun = TRUE;			/* this is a function call */
          DM_DBG(DM_N(4), "created new process\n");
        
          /* check variable table */  
          if(proc_fun.var_tab != NULL) {
            DM_ERR_ASSERT(_("proc_fun.var_tab != NULL\n"));
            state = sInit;
            continue;
          }
          proc_fun.var_tab = new Vars_t();
          DM_DBG(DM_N(2), "created local variable table %p for function `%s'\n", proc_fun.var_tab, nDefunNode->name->c_str());
        
          DM_DBG(DM_N(4), "gl_var_tab=%p, proc->var_tab=%p, proc_fun.var_tab=%p\n", &gl_var_tab, proc->var_tab, proc_fun.var_tab);
          
          /* pass arguments to the function by value (evaluate the arguments) */
          for(uint u = 0; u < args_size; u++) {
            proc_fun.WriteVariable(nDefunNode->params[u]->c_str(),
                                   Process::eval_str(args[u], proc).c_str(), FALSE);
          }
        
          if(proc_fun.n->child) {
            /* we have a function with non-empty body */
            Process proc_fun_body = Process(proc_fun.n->child, &proc_fun, NULL);
            DM_DBG(DM_N(4), "evaluating function %s\n", name.c_str());
            int fun_eval = proc_fun_body.eval();
            uint params_ref_size = nDefunNode->params_ref.size();
  
            /* simulate "passed by reference", i.e.: write into parent's scope */
            for(uint u = 0; u < params_ref_size; u++) {
              const char *v = proc_fun.ReadVariable(nDefunNode->params_ref[u]->c_str());
              if(v) {
                /* params/args_ref[u] is set */
                if(u != 0) s.push_back(' ');
                s.append(v);
              } else {
                /* unset */
                UPDATE_MAX(proc->executed, ERR_WARN);
                DM_WARN(ERR_WARN, _("variable `%s' is unset\n"), v);
              }
            }
            
            UPDATE_MAX(proc->executed, fun_eval);
            proc->evaluated = proc->executed;		/* for ${?} */
          }
          /* cleanup */
          DELETE(proc_fun.var_tab);
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(dq_param(name.c_str(), TRUE));
          for(uint u = 0; u < args_size; u++) {
            s.append(' ' + dq_param(args[u]->c_str(), TRUE));
          }
          s.append("}");
        }
        
        DELETE_VEC(args);

        state = sInit;
      }
      continue;

      case sMap:{
        TFunctions::iterator iter;		/* name/function object pair */
        const char *target_cstr = target.c_str();
        nDefun *nDefunNode = NULL;
        Process proc_fun;
        int chars;
        uint j;
        std::string name;
        std::vector <std::string *> args;	/* vector of call by value arguments */

        chars = get_dq_param(name, target_cstr);/* function name */
        name = eval_str(name.c_str(), proc);	/* evaluate things like: $MAP{...${var}... <arguments>} */

        DM_DBG(DM_N(4), "target=|%s|\n", target.c_str());
        DM_DBG(DM_N(4), "function name=|%s|\n", name.c_str());
        if(!nDefunNode) {
          if((iter = gl_fun_tab.find(name.c_str())) == gl_fun_tab.end()) {
            /* Function `name' is not defined. */
            UPDATE_MAX(proc->executed, ERR_ERR);
            DM_ERR(ERR_ERR, _("function `%s' not defined\n"), name.c_str());
            state = sInit;
            continue;
          }
        
          nDefunNode = iter->second;
        }
      
        /* get the number of parameters */
        uint params_size = nDefunNode->params.size();

        int l = chars;
        BOOL ws_only = FALSE;
        uint j_max = params_size? params_size: 1;

        if(proc->et != EVAL_ALL) {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(dq_param(name.c_str(), TRUE));
        }
        
        while(!ws_only) {
          args.clear();
          for(j = 0; j < j_max; j++) {
            int chars;
            std::string arg;
            chars = get_dq_param(arg, target_cstr + l, ws_only);
            arg = eval_str(arg.c_str(), proc);	/* evaluate things like: $MAP{name ...${var}...} */
            DM_DBG(DM_N(4), "arg=|%s|\n", arg.c_str());
            if(ws_only) break;
            std::string *new_arg = new std::string(arg);
            if(new_arg == (std::string *)NULL) {
              DM_ERR(ERR_SYSTEM, _("new failed\n"));
              state = sInit;
              continue;
            }
            args.push_back(new_arg);
            l += chars;
          }
          if(j == 0) 
            /* no more arguments */
            break;
  
          /* prepare for evaluation */
          uint args_size = args.size();
          
          if(proc->et == EVAL_ALL) {
            DM_DBG(DM_N(4), "args_size=%d; params_size=%d\n", args_size, params_size);
            if(args_size != params_size)
            {
              UPDATE_MAX(proc->executed, ERR_ERR);
              DM_ERR(ERR_ERR, _("number of function `%s' parameters (%u) != number of its arguments (%u) passed by value\n"), name.c_str(), params_size, args_size);
              state = sInit;
              continue;
            }
            
            DM_DBG(DM_N(4), "creating new process\n");
            proc_fun = Process(nDefunNode, proc, NULL);
//            proc_fun.fun = TRUE;			/* this is a function call */
            DM_DBG(DM_N(4), "created new process\n");
          
            /* check variable table */  
            if(proc_fun.var_tab != NULL) {
              DM_ERR_ASSERT(_("proc_fun.var_tab != NULL\n"));
              state = sInit;
              continue;
            }
            proc_fun.var_tab = new Vars_t();
            DM_DBG(DM_N(2), "created local variable table %p for function `%s'\n", proc_fun.var_tab, nDefunNode->name->c_str());
          
            DM_DBG(DM_N(4), "gl_var_tab=%p, proc->var_tab=%p, proc_fun.var_tab=%p\n", &gl_var_tab, proc->var_tab, proc_fun.var_tab);
            
            /* pass arguments to the function by value (evaluate the arguments) */
            for(uint u = 0; u < args_size; u++) {
              proc_fun.WriteVariable(nDefunNode->params[u]->c_str(),
                                     Process::eval_str(args[u], proc).c_str(), FALSE);
            }
          
            if(proc_fun.n->child) {
              /* we have a function with non-empty body */
              Process proc_fun_body = Process(proc_fun.n->child, &proc_fun, NULL);
              DM_DBG(DM_N(4), "evaluating function %s\n", name.c_str());
              int fun_eval = proc_fun_body.eval();
              uint params_ref_size = nDefunNode->params_ref.size();
    
              /* simulate "passed by reference", i.e.: write into parent's scope */
              for(uint u = 0; u < params_ref_size; u++) {
                const char *v = proc_fun.ReadVariable(nDefunNode->params_ref[u]->c_str());
                if(v) {
                  /* params/args_ref[u] is set */
                  if(u != 0) s.push_back(' ');
                  s.append(v);
                } else {
                  /* unset */
                  UPDATE_MAX(proc->executed, ERR_WARN);
                  DM_WARN(ERR_WARN, _("variable `%s' is unset\n"), v);
                }
              }
              
              UPDATE_MAX(proc->executed, fun_eval);
              proc->evaluated = proc->executed;		/* for ${?} */
            }
            /* cleanup */
            DELETE(proc_fun.var_tab);
          } else {
            for(uint u = 0; u < args_size; u++) {
              s.append(' ' + dq_param(args[u]->c_str(), TRUE));
            }
          }
          DELETE_VEC(args);
        }
        
        if(proc->et != EVAL_ALL) {
          s.append("}");
        }
        
        state = sInit;
      }
      continue;

      case sInterleave:{
        const char *target_cstr = target.c_str();
        int chars;
        std::string count;
        int64_t i64count;
        std::vector <std::string *> args;	/* vector of call by value arguments */

        chars = get_dq_param(count, target_cstr);	/* interleave count */
        i64count = Expr::eval2i(count.c_str(), proc);	/* evaluate things like: $INTERLEAVE{...${var}... <arguments>} */

        DM_DBG(DM_N(4), "target=|%s|\n", target.c_str());
        DM_DBG(DM_N(4), "i64count=%u\n", i64count);

        int l = chars;
        while(1) {
          BOOL ws_only;
          std::string arg;
          chars = get_dq_param(arg, target_cstr + l, ws_only);
          /* do not evaluate arguments */
          DM_DBG(DM_N(4), "arg=|%s|\n", arg.c_str());
          if(ws_only) break;
          std::string *new_arg = new std::string(arg);
          if(new_arg == (std::string *)NULL) {
            DM_ERR(ERR_SYSTEM, _("new failed\n"));
            state = sInit;
            continue;
          }
          args.push_back(new_arg);
          l += chars;
        }

        int args_size = args.size();

        if(proc->et == EVAL_ALL) {
          BOOL appended = FALSE;
          int64_t w = args_size/i64count;
          int64_t d = args_size-w*i64count;
          if(d) DM_WARN(ERR_WARN, _("%s: discarding %"PRIi64" elements\n"), std::string(state_name[state]).c_str(), d);
          for(int j = 0; j < w; j++) {
            int ind;
            for(int i = 0; i < i64count && (ind = w*i+j) < args_size; i++) {
              DM_DBG(DM_N(4), "i=%d; j=%d; args[%d]=|%s|\n", i, j, ind, args[ind]->c_str());
              if(appended) s.push_back(' ');
              s.append(args[ind]->c_str());  
              appended = TRUE;
            }
          }
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(dq_param(count.c_str(), TRUE));
          for(int i = 0; i < args_size; i++) {
            s.append(' ' + dq_param(args[i]->c_str(), TRUE));
          }
          s.append("}");
        }
        
        DELETE_VEC(args);

        state = sInit;
      }
      continue;

      case sSeq:{
        const char *target_cstr = target.c_str();
        std::string arg_x, arg_y;
        int chars;
        int64_t x, y;
        Expr e;
        Attr a;
  
        chars = get_dq_param(arg_x, target_cstr);
        chars = get_dq_param(arg_y, target_cstr + chars);
        arg_x = eval_str(arg_x.c_str(), proc);	/* evaluate things like: $SEQ{...${var}...} */
        DM_DBG(DM_N(4), "seq.x=|%s|\n", arg_x.c_str());
        arg_y = eval_str(arg_y.c_str(), proc);	/* evaluate things like: $SEQ{...${var}...} */
        DM_DBG(DM_N(4), "seq.y=|%s|\n", arg_y.c_str());

        if(proc->et == EVAL_ALL) {
          x = Expr::eval2i(arg_x.c_str(), proc);
          y = Expr::eval2i(arg_y.c_str(), proc);

          int step = x < y? 1 : -1;
          int j = x - step;
          do {
            j += step;
            if(j != x) s.push_back(' ');
            s.append(i2str(j));
          } while(j != y);
        } else {
          s.append("$" + std::string(state_name[state]) + "{");
          s.append(dq_param(arg_x.c_str(), TRUE) + " ");
          s.append(dq_param(arg_y.c_str(), TRUE));
          s.append("}");
        }
  
        state = sInit;
      }
      continue;

    }

    /* no TAGs found, simply copy the characters */
    s.push_back(c);
  }

  DM_DBG(DM_N(6), "state=%d; eval_str=|%s|\n", state, s.c_str());

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
  DM_DBG(DM_N(5), "eval_str=|%s|\n", s.c_str());

  RETURN(s);

#undef BALLANCED_OPL
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
    DM_DBG(DM_N(2), _("returning default value 0"));\
    return 0;\
  }\
  str_val = eval_str(str, this);\
\
  return get_##u##int##s(str_val.c_str(), &endptr, TRUE);\
}
_EVAL2INT(,);
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
_EVAL2PINT(,);
_EVAL2PINT(,32);
_EVAL2PINT(,64);
_EVAL2PINT(u,32);
_EVAL2PINT(u,64);
#undef _EVAL2PINT

/*
 * Evaluate std::vector <std::string *> into std::vector <std::string *>.
 */
std::vector <std::string *>
Process::eval_vec_str(const std::vector <std::string *> &v)
{
  DM_DBG_I;
  std::vector <std::string *> ev;
  std::vector <std::string *>::const_iterator iter;

  for(iter = v.begin(); iter != v.end(); iter++) {
    if(*iter) {
      const char *s_cstr = (*iter)->c_str();
      std::string target = preeval_str(s_cstr);
      const char *target_cstr = target.c_str();
      DM_DBG(DM_N(4), "s_cstr=|%s|; target_cstr=|%s|\n", s_cstr, target_cstr);
      uint l = 0;
      while(1) {
        BOOL ws_only;
        int chars;
        std::string arg;
        DM_DBG(DM_N(4), "target_cstr=|%s|\n", target_cstr + l);
        chars = get_dq_param(arg, target_cstr + l, ws_only);
        DM_DBG(DM_N(4), "chars=%d\n", chars);
        if(ws_only) break;
        arg = Process::eval_str(arg.c_str(), this);
        DM_DBG(DM_N(4), "ev.push_back(%s)\n", arg.c_str());
        ev.push_back(new std::string(arg.c_str()));
        l += chars;
      }
    }
    else ev.push_back((std::string *)NULL);
  }

  RETURN(ev);
} /* eval_vec_str */

/*
 * Evaluate std::vector <std::string *>.
 */
#define _EVAL_VEC_INT(u,s,fp)\
std::vector <u##int##s##_t> \
Process::eval_vec_##u##int##s(const std::vector <std::string *> &v)\
{\
  std::vector <std::string *> vs = eval_vec_str(v);\
  std::vector <u##int##s##_t> ev;\
\
  for(uint c = 0; c < vs.size(); c++) {\
    if(vs[c]) {\
      u##int##s##_t i = eval2##u##int##s(vs[c]);\
      DM_DBG(DM_N(3), "evaluating[%u] to %"fp"\n", c, i);\
      ev.push_back(i);\
    } else {\
      DM_DBG(DM_N(3), "default evaluation[%u] to 0\n", c);\
      ev.push_back(0);\
    }\
  }\
\
  DELETE_VEC(vs);\
  return ev;\
}
_EVAL_VEC_INT(,,"ld");
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
  std::vector <std::string *> vs = eval_vec_str(v);\
  std::vector <u##int##s##_t *> ev;\
\
  for(uint c = 0; c < vs.size(); c++) {\
    u##int##s##_t *i = NULL;\
    if(vs[c]) {\
      p##u##int##s##_t p = eval2p##u##int##s(vs[c]);\
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
  DELETE_VEC(vs);\
  return ev;\
}
_EVAL_VEC_PINT(,,"ld");
_EVAL_VEC_PINT(,32,"ld");
_EVAL_VEC_PINT(,64,"lld");
_EVAL_VEC_PINT(u,32,"lu");
_EVAL_VEC_PINT(u,64,"llu");
#undef _EVAL_VEC_PINT

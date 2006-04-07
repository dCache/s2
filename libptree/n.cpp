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
#include "n.h"

#include "free.h"
#include "date.h"
#include "constants.h"
#include "i18.h"
#include "timeout.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "expr.h"
#include "printf.h"
#include "s2.h"			/* opts (s2 options) */
#include "str.h"
#include "io.h"                 /* file_ropen(), ... */
#include "thread_pool.h"

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <errno.h>		/* errno */
#include <signal.h>             /* signal() */
#include <unistd.h>		/* sleep() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */

#include <iostream>             /* std::string, cout, endl, ... */

//#define USE_PTHREAD_ATTR              /* SEGVs! */

using namespace std;

#define FBRANCH "branch %u (%d/%d): "
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
  Node *n;
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
 * Constructor
 */
Node::Node()
{
  init();
}

/*
 * Destructor (virtual) + recursive postorder traversal
 */
Node::~Node()
{
  DM_DBG_I;

  /* first free the children */
  if(this->child) DELETE(this->child);

  /* then parallel branches */
  if(this->par) DELETE(this->par);

  DM_DBG_O;
}

/*
 * Initialise the node.
 */
void
Node::init()
{
  OFFSET = 0;
  COND = S2_COND_NONE;
  REPEAT.type = S2_REPEAT_NONE;
  REPEAT.X = 0;
  REPEAT.Y = 0;
  REPEAT.I = 0;
  EVAL = opts.s2_eval;
  TIMEOUT = opts.s2_timeout;

  /* PCRE options */
  match_opt.px = MATCH_PX;
  match_opt.pcre = MATCH_PCRE;
  
  /* debugging information */
  row = 0;
  has_executed = FALSE;
  executed = ERR_OK;
  evaluated = ERR_OK;

  /* tree overhead */
  parent = NULL;
  child = NULL;
  par = NULL;
  rpar = NULL;
}

/*
 * Initialise the node based on the value of another node.
 */
void
Node::init(Node &node)
{
  OFFSET = node.OFFSET;
  COND = node.COND;
  REPEAT.type = node.REPEAT.type;
  REPEAT.X = node.REPEAT.X;
  REPEAT.Y = node.REPEAT.Y;
  REPEAT.I = node.REPEAT.X;
  EVAL = node.EVAL;
  TIMEOUT = node.TIMEOUT;

  /* PCRE options */
  match_opt.px = node.match_opt.px;
  match_opt.pcre = node.match_opt.pcre;

  /* debugging information */
  row = node.row;
  has_executed = node.has_executed;
  executed = node.executed;
  evaluated = node.evaluated;

  /* tree overhead */
  parent = NULL;
  child = NULL;
  par = NULL;
  rpar = NULL;
}

int
Node::threads_init(void)
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
Node::threads_destroy(void)
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
 * Returns
 *   NULL: if no node with offset `offset' is found in the `append/bottom-most' branch
 *         pointer to the first node with offset `offset' otherwise
 */
Node *
Node::get_node_with_offset(Node *ptr_node, const uint offset)
{
  if(ptr_node == NULL)
    return NULL;

last_par:  
  if(ptr_node->OFFSET == offset)
    return ptr_node;


  /* descend to the last node on this level */
  while(ptr_node->par)
  {
    /* the same level, no need to check */
    ptr_node = ptr_node->par;
  }
//  DM_DBG(DM_N(0),"ptr_node->OFFSET=%u; n->OFFSET=%u\n", ptr_node->OFFSET, node->OFFSET);
  /* descend one level */
  if(ptr_node->child) {
    ptr_node = ptr_node->child;
    goto last_par;
  };

  /* node with offset `offset' was not found */
  return NULL;
} /* get_node_with_offset */

/* find an appropriate node to attach this node to + do the attachent */
int
Node::append_node(Node *root_node, Node *n)
{
  Node *ptr_node = root_node;

  if(ptr_node == n) {
    DM_ERR_ASSERT("trying to append the root node to itself\n");
    return ERR_ASSERT;
  }

  if(ptr_node == NULL) {
    DM_ERR_ASSERT("the root node is NULL\n");
    return ERR_ASSERT;
  }

last_par:  
  /* descend to the last node on this level */
  while(ptr_node->par)
  {
    ptr_node = ptr_node->par;
  }
//  DM_DBG(DM_N(0),"ptr_node->OFFSET=%d; n->OFFSET=%d\n", ptr_node->OFFSET, n->OFFSET);
  if(ptr_node->OFFSET < n->OFFSET)
  { /* descend one level */
    if(ptr_node->child) {
      ptr_node = ptr_node->child;
      goto last_par;
    };
    /* found the furthermost child => append */
    ptr_node->child = n;
    n->parent = ptr_node;
  } else {
    if(ptr_node->OFFSET != n->OFFSET) {
      /* 8-{} this should have been caught earlier in OFFSET()! */
      DM_ERR_ASSERT(_("invalid indentation, ignoring node %p\n"), n);
      return ERR_ASSERT;
    }
    
    /* offsets are equal => append the node after the last one on this level */
    ptr_node->par = n;
    n->parent = ptr_node->parent;
    n->rpar = ptr_node;
  }
  
  return ERR_OK;
} /* append_node */

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
std::string
Node::dq_param(const char *s, BOOL quote)
{
  DM_DBG_I;
  std::stringstream ss;
  BOOL q;

  if(!quote) RETURN(std::string(s));

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  q = str_char(s, ' ') != NULL ||       /* constains a space */
      str_char(s, '\t') != NULL ||      /* constains a tabulator */
      str_char(s, '\n') != NULL ||      /* constains a newline character */
      str_char(s, '\r') != NULL ||      /* constains a carriage return */
      *s == 0;                          /* empty string */

  if(q) ss << '"';

  ss << escape_chars(s, '"', q);

  if(q) ss << '"';

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

#undef DELIMIT_PARAM    /* for debugging only */

  RETURN(ss.str());
} /* print_dq_param */

std::string
Node::dq_param(const std::string &s, BOOL quote)
{
  if(!quote) return s.c_str();

  return dq_param(s.c_str(), quote);
}

std::string
Node::dq_param(const std::string *s, BOOL quote)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  if(!quote) return s->c_str();

  return dq_param(s->c_str(), quote);
}

std::string
Node::dq_param(const bool b, BOOL quote)
{
  std::string s = b? std::string("1"): std::string("0");
  
  return s;
}

std::string
Node::dq_param(const unsigned char c, BOOL quote)
{
  std::string s = i2str(c);
  
  return s;
}

/* 
 * Create a string parameter enclosed by double quotes depending on
 * characters in the string and its length.
 */
std::string
Node::ind_param(const char *s)
{
  std::stringstream ss;

//#define DELIMIT_PARAM
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  if(s == NULL) {
    ss << S2_NULL_STR;
    goto out;
  }

  ss << escape_chars(s, ']', TRUE);

out:
#ifdef DELIMIT_PARAM    /* for debugging only */
  ss << '|';
#endif

  return ss.str();

#undef DELIMIT_PARAM    /* for debugging only */
} /* ind_param */

std::string
Node::ind_param(const std::string &s)
{
  return ind_param(s.c_str());
}

std::string
Node::ind_param(const std::string *s)
{
  if(s == NULL) return std::string(S2_NULL_STR);

  return ind_param(s->c_str());
}

std::string
Node::nodeToString(uint indent, BOOL eval)
{
/* a pretty-printer macro to decide on printing default values */
#define IS_DEFAULT(v,def_val_indicator)\
  (!(opts.show_defaults) && (v) == (def_val_indicator))

  std::stringstream ss;
  int i;

  /* offset */
  i = indent;
  while(i-- > 0)
    ss << ' ';

  /* branch options */
  switch(COND) {
    case S2_COND_NONE: break;
    case S2_COND_OR: ss << "|| "; break;
    case S2_COND_AND: ss << "&& "; break;
  }

  if(REPEAT.type != S2_REPEAT_NONE) {
    ss << '>' << REPEAT.X;
    switch(REPEAT.type) {
      case S2_REPEAT_NONE: DM_ERR_ASSERT(_("S2_REPEAT_NONE\n")); break;
      case S2_REPEAT_OR: ss << "||"; break;
      case S2_REPEAT_AND: ss << "&&"; break;
      case S2_REPEAT_PAR: ss << ' '; break;
    }
    ss << REPEAT.Y << ' ';
  }

  if(!IS_DEFAULT(EVAL, S2_EVAL))
    ss << "eval=" << EVAL << ' ';

  if(!IS_DEFAULT(TIMEOUT, S2_TIMEOUT))
    ss << "timeout=" << TIMEOUT << ' ';

  if(!(IS_DEFAULT(match_opt.px, MATCH_PX) && 
       IS_DEFAULT(match_opt.pcre, MATCH_PCRE)))
  {
    /* we have non-default match options => print them */
    char *cstr = get_match_opts(match_opt);
    ss << "match=";
    ss << cstr; FREE(cstr);
    ss << ' ';
  }

  /* the action itself */
  ss << toString(eval);

  return ss.str();

#undef IS_DEFAULT
}

int
Node::print_node
(uint indent, FILE *file, BOOL eval, BOOL show_executed, BOOL show_evaluated)
{
  if(show_executed && !has_executed)
    /* don't show node which hasn't executed */
    return ERR_OK;

  S_P(&thread.print_mtx);
  if(has_executed && show_executed) {
    if(show_evaluated)
      fprintf(file, "%d/%d: ", executed, evaluated);
    else
      fprintf(file, "%d: ", executed);
  }

  fputs(nodeToString(indent, eval).c_str(), file);
  fputc('\n', file);
  S_V(&thread.print_mtx);

  return ERR_OK;
} /* print_node */

/* 
 * Recursive preorder traversal of the tree.
 */
int
Node::print_tree(uint indent, FILE *file, BOOL pp_indent, BOOL eval, BOOL exec_eval)
{
  print_node(pp_indent? indent: OFFSET, file, eval, exec_eval, exec_eval);

  /* first print the children */
  if(child) {
    child->print_tree(pp_indent? (indent+opts.pp_indent): OFFSET, file, pp_indent, eval, exec_eval);
  }

  /* then parallel branches */
  if(par) {
    par->print_tree(pp_indent? indent: OFFSET, file, pp_indent, eval, exec_eval);
  }
  
  return ERR_OK;
} /* print_tree */

/*
 * For use in threads only.
 */
void *
eval_in_parallel(void *node)
{
  DM_DBG_I;

  Node *n = (Node *)node;
  int root_eval;
  
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", n->row, n->executed, n->evaluated, n);
  DM_DBG(DM_N(3), "%s\n", n->toString(FALSE).c_str());

  root_eval = n->eval_sequential_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, n->evaluated, root_eval);
  MUTEX(&thread.total_mtx, thread.total--);

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", n->row, n->executed, n->evaluated, pthread_self());
//  if(n->TIMEOUT) timeout_del(pthread_self());

#if 0
  pthread_exit((void *)node);
#else
  return (void *)node;
#endif
} /* eval_in_parallel */

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
  Node *n;
  
  if(p_request == NULL) {
    DM_ERR_ASSERT("request == NULL\n");
    RETURN(NULL);
  }
  n = (Node *)p_request->data;

  DM_DBG(DM_N(3), "node=%p, thread (%d)\n", n, p_request->tp_tid);

  eval_in_parallel(n);

  DM_DBG(DM_N(3), "node=%p, thread (%d): executed=%d; evaluated=%d\n",
         n, p_request->tp_tid, n->executed, n->evaluated);
  
  RETURN(NULL);
}

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Node::eval()
{
  DM_DBG_I;
  std::string str_node = nodeToString(OFFSET, TRUE);
  DM_DBG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_LOG_B(DM_N(1), "%s\n", str_node.c_str());
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", row, executed, evaluated, this);

  int root_eval, par_eval;
  int sreqs = 0;	/* number of parallel subrequests created */
  pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
  pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;

  /* Schedule parallel execution */
  if(par) {
    Node *ptr_node;
 
    DM_DBG(DM_N(3), FBRANCH"branches located at the same offset exist\n", row, executed, evaluated);

    /* check for parallel execution and create new requests if parallel branches found */
    for(ptr_node = par; ptr_node; ptr_node = ptr_node->par) {
      BOOL can_enqueue;
      can_enqueue = FALSE;
      
      if(ptr_node->COND == S2_COND_NONE) {
        if(ptr_node->REPEAT.type == S2_REPEAT_PAR) {
          /* repeats execution */
          ptr_node->eval_repeats();
          continue;
        }
        
        S_P(&thread.total_mtx);
        if(thread.total < opts.tp_size) {
          thread.total++;
          can_enqueue = TRUE;
        }
        S_V(&thread.total_mtx);
        
        if(can_enqueue) {
          DM_DBG(DM_N(1), FBRANCH"enqueing a request\n", row, executed, evaluated);
          if(tp_enqueue(ptr_node, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", row, executed, evaluated, sreqs);
            sreqs++;
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", row, executed, evaluated, thread.total, opts.tp_size);
          root_eval = ptr_node->eval_repeats();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, root_eval);
        }
      }
    }
  } else {
    DM_DBG(DM_N(3), FBRANCH"no branches located at the same offset exist\n", row, executed, evaluated);
  }

  root_eval = eval_repeats();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, root_eval);
  DM_DBG(DM_N(3), FBRANCH"root_eval=%d\n", row, executed, evaluated, root_eval);

  /* Investigate branches at the same offset */
  if(par) {
    Node *ptr_node = par;
    DM_DBG(DM_N(5), FBRANCH"investigating branches located at the same offset\n", row, executed, evaluated);
    switch(par->COND) {
      case S2_COND_OR:
        if(root_eval > EVAL) {
          DM_DBG(DM_N(5), FBRANCH"OR: root_eval=%d > EVAL=%d\n", row, executed, evaluated, root_eval, EVAL);
          par_eval = par->eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", row, executed, evaluated, par_eval);
        } else {
          /* see if we have an AND node that we need to evaluate */
          DM_DBG(DM_N(5), FBRANCH"OR: evaluated(%d) <= EVAL(%d)\n", row, executed, evaluated, evaluated, EVAL);

          ptr_node = par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_AND) goto eval_and;
            ptr_node = ptr_node->par;
          }
          break;

eval_and:
          DM_DBG(DM_N(5), FBRANCH"found a par AND, evaluating branch %u\n", row, executed, evaluated, ptr_node->row);
          par_eval = ptr_node->eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"OR: par_eval=%d\n", row, executed, evaluated, par_eval);
        }
      break;

      case S2_COND_AND:
        if(root_eval <= EVAL) {
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) <= EVAL(%d)\n", row, executed, evaluated, root_eval, EVAL);
          par_eval = par->eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", row, executed, evaluated, par_eval);
        } else {
          /* see if we have an OR node that can save us (return TRUE evaluation) */
          DM_DBG(DM_N(5), FBRANCH"AND: root_eval(%d) > EVAL(%d)\n", row, executed, evaluated, root_eval, EVAL);

          ptr_node = par->par;
          while(ptr_node) {
            if(ptr_node->COND == S2_COND_OR) goto eval_or;
            ptr_node = ptr_node->par;
          }
          break;

eval_or:
          DM_DBG(DM_N(5), FBRANCH"found a par OR, evaluating branch %u\n", row, executed, evaluated, ptr_node->row);
          par_eval = ptr_node->eval();
          S_P(&thread.evaluated_mtx);
          evaluated = par_eval;
          S_V(&thread.evaluated_mtx);
          DM_DBG(DM_N(5), FBRANCH"AND: par_eval=%d\n", row, executed, evaluated, par_eval);
        }
      break;
      
      case S2_COND_NONE:
        /* parallel execution, taken care of by a new thread scheduled at the top */
        DM_DBG(DM_N(5), FBRANCH"found a parallel branch, execution already scheduled\n", row, evaluated, executed);
      break;
    }
  }

  S_P(&sreqs_mtx);
  while(sreqs != 0) {
    DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", row, executed, evaluated, sreqs);
    pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
  }
  S_V(&sreqs_mtx);

  /* threads-related cleanup */
  pthread_cond_destroy(&sreqs_cv);
  pthread_mutex_destroy(&sreqs_mtx);

  /* Parallel execution finished, we have one thread of execution. *
   * Go through the parallel branches and set the return value.    */
  if(par) {
    Node *ptr_node;
    for(ptr_node = par; ptr_node; ptr_node = ptr_node->par)
      if(ptr_node->COND == S2_COND_NONE)
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, ptr_node->evaluated);
  }

  DM_DBG(DM_N(2), FBRANCH"complete evaluation=%d\n", row, executed, evaluated, root_eval);
  DM_LOG(DM_N(2), FBRANCH"complete evaluation=%d\n", row, executed, evaluated, root_eval);
  RETURN(evaluated);
} /* eval */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Node::eval_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", row, executed, evaluated, this);

  int repeats_eval;

  switch(REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_OR:		/* fall through */
    case S2_REPEAT_AND:		/* fall through */
      repeats_eval = eval_sequential_repeats();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, repeats_eval);
      DM_DBG(DM_N(5), FBRANCH"repeats_eval=%d\n", row, executed, evaluated, repeats_eval);
    break;

    case S2_REPEAT_PAR:
      uint threads_total = 1 + ((REPEAT.X > REPEAT.Y)? REPEAT.X - REPEAT.Y: REPEAT.Y - REPEAT.X);
      int sreqs = 0;	/* number of parallel subrequests created */
      uint threads_i = 0;
      pthread_cond_t sreqs_cv = PTHREAD_COND_INITIALIZER;
      pthread_mutex_t sreqs_mtx = PTHREAD_MUTEX_INITIALIZER;
      DM_DBG(DM_N(1), _("parallel repeat "FBRANCH"total number of threads=%d\n"), row, executed, evaluated, threads_total);
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
          DM_DBG(DM_N(1), "parallel repeat "FBRANCH"enqueing a request\n", row, executed, evaluated);
          if(tp_enqueue(this, &sreqs, &sreqs_cv)) {
            DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create new thread: %s\n"), row, executed, evaluated, strerror(errno));
          } else {
            /* number of sub-requests */
            DM_DBG(DM_N(3), FBRANCH"subrequests=%d\n", row, executed, evaluated, sreqs);
            sreqs++;
          }
        } else {
          /* too many parallel requests, could lead to a deadlock on pthread_cond_wait()
             => evaluate sequentially */
          DM_DBG(DM_N(3), FBRANCH"too many parallel requests (%d >= %d), evaluating sequentially\n", row, executed, evaluated, thread.total, opts.tp_size);
          repeats_eval = eval_with_timeout();
          UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, repeats_eval);
        }
      }

      S_P(&sreqs_mtx);
      while(sreqs != 0) {
        DM_DBG(DM_N(2), FBRANCH"waiting for sreqs=0 (%d)\n", row, executed, evaluated, sreqs);
        pthread_cond_wait(&sreqs_cv, &sreqs_mtx);
      }
      S_V(&sreqs_mtx);

      /* threads-related cleanup */
      pthread_cond_destroy(&sreqs_cv);
      pthread_mutex_destroy(&sreqs_mtx);

    break;
  }

  DM_DBG(DM_N(5), FBRANCH"eval_repeats returns\n", row, executed, evaluated);
  RETURN(evaluated);
} /* eval_repeats */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Node::eval_sequential_repeats()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", row, executed, evaluated, this);

  int8_t step = REPEAT.X < REPEAT.Y? 1 : -1;
  int iter_eval;
  REPEAT.I = REPEAT.X - step;

  switch(REPEAT.type) {
    case S2_REPEAT_NONE:	/* fall through */
    case S2_REPEAT_PAR:		/* fall through: parallelism already handled in eval_repeats() */
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"\n", row, executed, evaluated);
      iter_eval = eval_with_timeout();
      UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
      DM_DBG(DM_N(5), "NO/PAR repeat "FBRANCH"iter_eval=%d\n", row, executed, evaluated, iter_eval);
    break;

    case S2_REPEAT_OR:
      while(REPEAT.I != REPEAT.Y) {
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"REPEAT.I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", row, evaluated, executed, REPEAT.I, REPEAT.Y);
        REPEAT.I += step;

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
        DM_DBG(DM_N(5), "OR repeat "FBRANCH"iter_eval=%d\n", row, evaluated, executed, iter_eval);
        if(evaluated <= EVAL) {
          DM_DBG(DM_N(5), "OR repeat "FBRANCH"successfully evaluated; evaluated=%d <= EVAL=%d\n", row, executed, evaluated, evaluated, EVAL);
          /* end on first successful evaluation */
          break;
        }
      }
    break;

    case S2_REPEAT_AND:
      while(REPEAT.I != REPEAT.Y) {
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"REPEAT.I=%"PRIi64" != REPEAT.Y=%"PRIi64"\n", row, executed, evaluated, REPEAT.I, REPEAT.Y);
        REPEAT.I += step;

        iter_eval = eval_with_timeout();
        UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, iter_eval);
        DM_DBG(DM_N(5), "AND repeat "FBRANCH"iter_eval=%d\n", row, executed, evaluated, iter_eval);
        if(evaluated > EVAL) {
          DM_DBG(DM_N(5), "AND repeat "FBRANCH"unsuccessfully evaluated; evaluated=%d > EVAL=%d\n", row, executed, evaluated, evaluated, EVAL);
          /* end on first unsuccessful evaluation */
          break;
        }
      }
    break;

  }

  DM_DBG(DM_N(5), FBRANCH"eval_sequential_repeats returns\n", row, executed, evaluated);
  RETURN(evaluated);
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
  
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", ti->n->row, ti->n->executed, ti->n->evaluated, ti->n);
  DM_DBG(DM_N(3), "%s\n", ti->n->toString(FALSE).c_str());

  root_eval = ti->n->eval_without_timeout();
  UPDATE_MAX_MUTEX(&thread.evaluated_mtx, ti->n->evaluated, root_eval);
  ti->terminated = TRUE;

  DM_DBG(DM_N(3), FBRANCH"quitting thread id %lu\n", ti->n->row, ti->n->executed, ti->n->evaluated, pthread_self());
  
  if(pthread_cond_broadcast(&thread.timeout_cv)) {
    DM_ERR(ERR_SYSTEM, _("pthread_cond_signal failed: %s\n"), strerror(errno));
  }

#if 0
  pthread_exit((void *)ti->n);
#else
  return (void *)ti->n;
#endif
} /* eval_in_parallel_notimeout */

/* 
 * Evaluate nodes of the tree by recursive preorder traversal.
 */
int
Node::eval_with_timeout()
{
  DM_DBG_I;
  DM_DBG(DM_N(3), FBRANCH"node=%p\n", row, executed, evaluated, this);

  pthread_t thread_id;
  struct timeval now;			/* timeval uses micro-seconds */
  struct timespec timeout;		/* timespec uses nano-seconds */
  timeout_info_t ti;

  /* disable timeouts until they're properly implemented and tested */
  RETURN(eval_without_timeout());

  if(!TIMEOUT) {
    /* no timeout needed */
    DM_DBG(DM_N(3), FBRANCH"no timeout set\n", row, executed, evaluated);
    RETURN(eval_without_timeout());
  }

  /* set thread cleanup handler */
  DM_DBG(DM_N(3), FBRANCH"pushing cleanup handler\n", row, executed, evaluated);
  pthread_cleanup_push(pthread_timeout_handler, (void*)this);

  ti.n = this;
  ti.terminated = FALSE;
  
  /* timeouts handling */
  DM_DBG(DM_N(3), FBRANCH"node=%p; creating new thread with timeout=%"PRIu64"\n", row, executed, evaluated, this, TIMEOUT);
  int thread_rval = thread_create(&thread_id, eval_in_parallel_without_timeout, &ti);
  if(thread_rval)
  {
    DM_ERR(ERR_SYSTEM, _(FBRANCH"failed to create thread: %s\n"), row, executed, evaluated, strerror(errno));
  } else {
    /* thread was created fine */
    DM_DBG(DM_N(3), FBRANCH"thread %lu created successfully, adding timeout=%"PRIu64"\n", row, executed, evaluated, thread_id, TIMEOUT);
    
    if(gettimeofday(&now, NULL)) {
      DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
      RETURN(ERR_SYSTEM);
    }
    unsigned long timeout_add_sec = TIMEOUT / 1000000;
    unsigned long timeout_add_usec = TIMEOUT - ((TIMEOUT / 1000000) * 1000000);

    timeout.tv_sec = now.tv_sec + timeout_add_sec;
    timeout.tv_nsec = (now.tv_usec + timeout_add_usec) * 1000;

    S_P(&thread.timeout_mtx);
    DM_DBG_T(DM_N(2), FBRANCH"waiting for timeout_cv\n", row, executed, evaluated);
    int rc;
    while(1) {
      rc = pthread_cond_timedwait(&thread.timeout_cv, &thread.timeout_mtx, &timeout);
      if(rc == ETIMEDOUT) {
        /* timeout reached, cancel the thread */
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv timed out\n", row, executed, evaluated);
        break;
      }
      if(ti.terminated) {
        DM_DBG_T(DM_N(2), FBRANCH"timeout_cv triggered\n", row, executed, evaluated);
        break;
      }
    }
    S_V(&thread.timeout_mtx);

    if(rc == ETIMEDOUT) {
      /* timeout reached, cancel the thread */
      DM_DBG_T(DM_N(2), FBRANCH"cancelling thread %lu\n", row, executed, evaluated, thread_id);
      pthread_cancel(thread_id);
    }
  }

  /* ``reap'' the thread */
  Node *n;            /* pointer to a return value from a thread */
  DM_DBG(DM_N(3), FBRANCH"reaping thread %lu\n", row, executed, evaluated, thread_id);
  if(thread_join(thread_id, (void **)&n)) {
    DM_ERR(ERR_SYSTEM, FBRANCH"failed to join thread: %s\n", row, executed, evaluated, strerror(errno));
  }
  DM_DBG(DM_N(3), FBRANCH"joined thread; node ptr=%p\n", row, executed, evaluated, n);
  if(n != PTHREAD_CANCELED) {
    DM_DBG(DM_N(3), FBRANCH"exited with value %d\n", row, executed, evaluated, n->evaluated);
  } else {
    /* timeout */
    int root_eval;
    DM_DBG(DM_N(2), FBRANCH"timeout reached\n", row, executed, evaluated);
    DM_LOG(DM_N(2), FBRANCH"timeout reached\n", row, executed, evaluated);
    root_eval = ERR_NEXEC;
    UPDATE_MAX_MUTEX(&thread.evaluated_mtx, evaluated, ERR_NEXEC);
    if(opts.e1_fname) print_node(OFFSET, opts.e1_file, TRUE, TRUE, FALSE);
  }

  /* Remove thread cleanup handler. */
  pthread_cleanup_pop(0);

  RETURN(evaluated);
} /* eval_with_timeout */

/* 
 * Evaluate subtree of a node.
 */
int
Node::eval_without_timeout()
{
  int no_timeout_exec, no_timeout_eval;

  if(opts.e0_fname) print_node(OFFSET, opts.e0_file, TRUE, FALSE, FALSE);
  DM_DBG_T(DM_N(4), FBRANCH"starting execution of node=%p\n", row, executed, evaluated, this);
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(OFFSET, TRUE).c_str());
  has_executed = TRUE;		/* don't move this behind exec(), timeouts! */
  no_timeout_exec = exec();
//  DM_LOG_B(DM_N(1), "%s\n", nodeToString(OFFSET, TRUE).c_str());
  S_P(&thread.evaluated_mtx);
  executed = evaluated = no_timeout_exec;	/* for ${?} */
  S_V(&thread.evaluated_mtx);
  if(opts.e1_fname) print_node(OFFSET, opts.e1_file, TRUE, TRUE, FALSE);
  DM_DBG_T(DM_N(4), FBRANCH"finished execution (of node=%p)\n", row, executed, evaluated, this);
  DM_LOG(DM_N(2), FBRANCH"executed=%d\n", row, executed, evaluated, no_timeout_exec);

  no_timeout_eval = eval_subtree(no_timeout_exec, no_timeout_exec);
  DM_DBG(DM_N(4), FBRANCH"node=%p; no_timeout_eval=%d\n", row, executed, evaluated, this, no_timeout_eval);

  S_P(&thread.evaluated_mtx);
  evaluated = no_timeout_eval;
  S_V(&thread.evaluated_mtx);
  return no_timeout_eval;
}

/* 
 * Evaluate subtree of a node.
 */
int
Node::eval_subtree(const int root_exec, int &root_eval)
{
  int child_eval; 

  /* investigate CHILDREN */
  if(root_exec <= EVAL) { 
    /* evaluate the children (only if root executed fine) */
    if(child) {
      DM_DBG(DM_N(3), FBRANCH"evaluating child branch %u\n", row, executed, evaluated, child->row);
      child_eval = child->eval();
      root_eval = child_eval;
    }
  }

  DM_DBG(DM_N(5), FBRANCH"subtree_eval=%d\n", row, executed, evaluated, root_eval);
  return root_eval;
}
/* eval_subtree */

/*
 * Evaluate the expected string before PCRE matching.
 */
BOOL
Node::e_match(const std::string *expected, const char *received)
{
  BOOL match;
  std::string s_expected;

  if(expected == NULL || received == NULL) {
    DM_DBG(DM_N(1), "expected == %p || received == %p\n", expected, received);
    RETURN(!(match_opt.pcre & PCRE_NOTEMPTY));
  }

  s_expected = eval_str(expected, TRUE);
  match = pcre_matches(s_expected.c_str(), received, match_opt.pcre, WriteVariable);
  
  return match;
} /* e_match */

BOOL
Node::e_match(const char *expected, const char *received)
{
  BOOL match;
  std::string s_expected = eval_str(expected, TRUE);
  match = pcre_matches(s_expected.c_str(), received, match_opt.pcre, WriteVariable);

  return match;
} /* e_match */

/*** Private functions **********************************************/

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
Node::eval_str(const char *cstr, BOOL eval)
{
  int i, c;
  int slen;
  BOOL bslash = FALSE;  /* we had the '\\' character */
  std::string s;
  std::string var;
  enum s_eval { 
    sInit, sDollar, sVar, sEvar, sCounter, sExpr,  sRandom,  sDate,  sPrintf,
  } state = sInit;
  static const char* state_name[] = {
    "0",   "",      "",   "ENV", "I",      "EXPR", "RANDOM", "DATE", "PRINTF",
  };
  const char *opt;
  int opt_off;
  int brackets = 0;

  if(cstr == NULL) {
    DM_ERR_ASSERT(_("c_str == NULL\n"));
    return std::string(S2_NULL_STR);
  }

  if(!eval) return std::string(cstr);

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
        } else if(OPL("RANDOM{")) {
          /* random values */
          var.clear();
          state = sRandom;
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
            int parent_executed = parent? parent->executed: 0;
            DM_DBG(DM_N(4), "returning parent's execution value %d\n", parent_executed);
            s.append(i2str(parent_executed));
            state = sInit;
            continue;
          }
	  
          if(var == "!") {	/* ${!} */
            /* return evaluation value of the branch at the same offset */
            int rpar_evaluated = rpar? rpar->evaluated: 0;
            DM_DBG(DM_N(4), "returning rpar evaluation value %d\n", rpar_evaluated);
            s.append(i2str(rpar_evaluated));
            state = sInit;
            continue;
          }
	  
          /* classic variable ${var} */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: ${...${var}...} */

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
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $ENV{...${var}...} */

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
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $I{...${var}...} */

          const char *word;
          word = var.c_str();
          char *endptr;
          int16_t i16;

          i16 = get_int16(word, &endptr, FALSE);
          if(endptr != word) {
            int64_t i64 = 0;
            Node *ptr_node = this;
            while(ptr_node && i16 >= 0) {
              if(ptr_node->REPEAT.type == S2_REPEAT_OR || 
                 ptr_node->REPEAT.type == S2_REPEAT_AND)
              {
                DM_DBG(DM_N(4), "found OR or AND repeat; i16=%d\n", i16);
                if(!i16) {
                  i64 = ptr_node->REPEAT.I;
                  DM_DBG(DM_N(4), "$I{%d}=%"PRIi64"\n", i16, i64);
                  break;
                } 
                i16--;
              }
              ptr_node = ptr_node->parent;
            }
            if(ptr_node == NULL)
              DM_WARN(ERR_WARN, _(FBRANCH"couldn't find repeat loop for repeat counter $I{%s}!\n"), row, executed, evaluated, word);

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
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $EXPR{...${var}...} */

          int64_t e = 0;
          DM_DBG(DM_N(4), "expr=|%s|\n", var.c_str());
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), row, executed, evaluated, var.c_str());
          } else s.append(i2str(e));
          state = sInit;
        } else {
          var.push_back(c);
        }
      }
      continue;

      case sRandom:{
        if(c == '}' && !brackets) {
          /* we have a maximum+1 random number */
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $RANDOM{...${var}...} */

          int64_t e = 0;
          if(str_expr2i(var.c_str(), &e)) {
            DM_ERR(ERR_ERR, _(FBRANCH"couldn't evaluate expression `%s'\n"), row, executed, evaluated, var.c_str());
          } else {
            srandom(time(NULL));
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
            date = ssprintf("%04d-%02d-%02d@%02d:%02d:%02d.%06lu ",
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
        if(c == '}' && !brackets) { // TODO:
          char *argv;
          char **arg;
          int spaces = 0;
          int j, plen;
          const char *var_cstr;
          var = eval_str(var.c_str(), eval);	/* evaluate things like: $PRINTF{...${var}...} */
          var_cstr = var.c_str();

          plen = strlen(var_cstr);
          for(j = 0; j < plen; j++) {
            if(IS_WHITE(argv[j])) spaces++;
          }
          if((arg = (char **)malloc(sizeof(char *) * (spaces + 2))) == (char **)NULL) {
            DM_ERR(ERR_SYSTEM, _("malloc failed\n"));
            return s;
          }
          std::string target = " ";
          int l = 0;
          for(j = 0; target.length() != 0; j++) {
            get_dq_param(target, var_cstr + l);
            if((arg[j] = (char *)strndup(var_cstr, l + target.length()))) {
              DM_ERR(ERR_SYSTEM, _("strdup failed\n"));
              return s;
            }
            l += target.length();
            fprintf(stderr, "%s\n", arg[0]);
            fprintf(stderr, "%s\n", arg[1]);
            fprintf(stderr, "%s\n", arg[2]);
          }
          arg[++j] = 0;

          var = eval_str(var.c_str(), eval);	/* evaluate things like: $RANDOM{...${var}...} */
          s.append(ssprintf_chk(arg));
          FREE(arg);
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
Node::eval_str(const std::string *s, BOOL eval)
{
  if(s == NULL) {
    DM_ERR_ASSERT(_("s == NULL\n"));
    return std::string(S2_NULL_STR);
  }
  return eval_str(s->c_str(), eval);
} /* eval_str */

#define _EVAL2INT(u,s)\
u##int##s##_t \
Node::eval2##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
\
  if(str == NULL) {\
    DM_DBG(DM_N(0), _("returning default value 0"));\
    return 0;\
  }\
  str_val = eval_str(str, TRUE);\
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
Node::eval2p##u##int##s(const std::string *str)\
{\
  std::string str_val;\
  char *endptr;\
  p##u##int##s##_t r;\
\
  if(str == NULL) goto ret_null;\
  str_val = eval_str(str, TRUE);\
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
Node::eval_vec_str(const std::vector <std::string *> &v, BOOL eval)
{
  std::vector <std::string *> ev;

  for(uint i = 0; i < v.size(); i++) {
    if(v[i]) ev.push_back(new std::string(eval_str(v[i], eval).c_str()));
    else ev.push_back((std::string *)NULL);
  }

  return ev;
} /* eval_vec_str */

/*
 * Evaluate std::vector <std::string *>.
 */
#define _EVAL_VEC_INT(u,s,fp)\
std::vector <u##int##s##_t> \
Node::eval_vec_##u##int##s(const std::vector <std::string *> &v, BOOL eval)\
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
Node::eval_vec_p##u##int##s(const std::vector <std::string *> &v, BOOL eval)\
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

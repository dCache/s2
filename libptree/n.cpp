#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "n.h"
#include "process.h"

#include "md5f.h"
#include "free.h"
#include "date.h"
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "s2.h"			/* opts (s2 options), progress(), ... */
#include "str.h"
#include "io.h"                 /* file_ropen(), ... */
#include "process.h"		/* S_P, S_V */
#include "thread_pool.h"	/* tp_sync_t */

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <errno.h>		/* errno */
#include <unistd.h>		/* sleep() */
#include <stdlib.h>             /* exit() */
#include <stdio.h>              /* stderr */

#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

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

  /* free dynamically allocated data of this node first */
  DELETE(REPEAT.X);
  DELETE(REPEAT.Y);
  
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
  TYPE = N_GENERAL;
  OFFSET = 0;
  COND = S2_COND_NONE;
  REPEAT.type = S2_REPEAT_NONE;
  REPEAT.X = NULL;
  REPEAT.Y = NULL;
  EVAL = opts.s2_eval;
  TIMEOUT = opts.s2_timeout;

  /* PCRE options */
  match_opt.px = MATCH_PX;
  match_opt.pcre = MATCH_PCRE;
  
  /* debugging/logging information (executed/evaluated _only_ for e2) */
  row = 0;
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
  TYPE = node.TYPE;
  OFFSET = node.OFFSET;
  COND = node.COND;
  REPEAT.type = node.REPEAT.type;
  REPEAT.X = node.REPEAT.X? new std::string(node.REPEAT.X->c_str()) : NULL;
  REPEAT.Y = node.REPEAT.Y? new std::string(node.REPEAT.Y->c_str()) : NULL;
  EVAL = node.EVAL;
  TIMEOUT = node.TIMEOUT;

  /* PCRE options */
  match_opt.px = node.match_opt.px;
  match_opt.pcre = node.match_opt.pcre;

  /* debugging/logging information (executed/evaluated _only_ for e2) */
  row = node.row;
  executed = node.executed;
  evaluated = node.evaluated;

  /* tree overhead */
  parent = NULL;
  child = NULL;
  par = NULL;
  rpar = NULL;
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

  DM_DBG(DM_N(4), "attaching node %p (root %p)\n", n, root_node);

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

  DM_DBG(DM_N(4), "attached node %p (branch=%u; parent=%p; rpar=%p)\n", n, n->row, n->parent, n->rpar);
  
  return ERR_OK;
} /* append_node */

std::string
Node::nodeToString(Node *n, uint indent, Process *proc)
{
/* a pretty-printer macro to decide on printing default values */
#define IS_DEFAULT(v,def_val_indicator)\
  (!(opts.show_defaults) && (v) == (def_val_indicator))

#define EXPR2I(s)\
  (proc? Process::eval_str(s, proc) : s->c_str())

  std::stringstream ss;
  int i;

  /* offset */
  i = indent;
  while(i-- > 0)
    ss << ' ';

  /* branch options */
  switch(n->COND) {
    case S2_COND_NONE: break;
    case S2_COND_OR: ss << "|| "; break;
    case S2_COND_AND: ss << "&& "; break;
  }

  switch(n->REPEAT.type) {
    case S2_REPEAT_NONE:
    break;

    case S2_REPEAT_OR:
      ss << '>' << EXPR2I(n->REPEAT.X) << "||" << EXPR2I(n->REPEAT.Y);
    break;

    case S2_REPEAT_AND:
      ss << '>' << EXPR2I(n->REPEAT.X) << "&&" << EXPR2I(n->REPEAT.Y);
    break;

    case S2_REPEAT_PAR:
      ss << '>' << EXPR2I(n->REPEAT.X) << ' '  << EXPR2I(n->REPEAT.Y);
    break;

    case S2_REPEAT_WHILE:
      ss << "WHILE";
    break;
  }
  if(n->REPEAT.type != S2_REPEAT_NONE) ss << ' ';

  if(!IS_DEFAULT(n->EVAL, S2_EVAL))
    ss << "eval=" << n->EVAL << ' ';

  if(!IS_DEFAULT(n->TIMEOUT, S2_TIMEOUT))
    ss << "timeout=" << n->TIMEOUT << ' ';

  if(!(IS_DEFAULT(n->match_opt.px, MATCH_PX) && 
       IS_DEFAULT(n->match_opt.pcre, MATCH_PCRE)))
  {
    /* we have non-default match options => print them */
    char *cstr = get_match_opts(n->match_opt);
    ss << "match=";
    ss << cstr; FREE(cstr);
    ss << ' ';
  }

  /* the action itself */
  ss << n->toString(proc);

  return ss.str();

#undef IS_DEFAULT
}

int
Node::print_node
(uint indent, FILE *file, Process *proc, BOOL show_executed, BOOL show_evaluated)
{
  Node::print_node(this, indent, file, proc, show_executed, show_evaluated);

  return ERR_OK;
} /* print_node */

int
Node::print_node
(Node *n, uint indent, FILE *file, Process *proc, BOOL show_executed, BOOL show_evaluated)
{
  int executed, evaluated;

  if(proc) {
    executed  = proc->executed;
    evaluated = proc->evaluated;
  } else {
    executed  = n->executed;
    evaluated = n->evaluated;
  }

  std::string str = nodeToString(n, indent, proc);

  S_P(&tp_sync.print_mtx,proc);
  if(show_executed) {
    if(show_evaluated)
      fprintf(file, "%d/%d:", executed, evaluated);
    else
      fprintf(file, "%d:", executed);
  }

  fputs(str.c_str(), file);
  fputc('\n', file);
  S_V(&tp_sync.print_mtx);

  return ERR_OK;
} /* print_node */

/* 
 * Recursive preorder traversal of the tree.
 */
int
Node::print_tree(uint indent, FILE *file, BOOL pp_indent, BOOL exec_eval)
{
  print_node(pp_indent? indent: OFFSET, file, NULL, exec_eval, exec_eval);

  /* first print the children */
  if(child) {
    child->print_tree(pp_indent? (indent+opts.pp_indent): OFFSET, file, pp_indent, exec_eval);
  }

  /* then parallel branches */
  if(par) {
    par->print_tree(pp_indent? indent: OFFSET, file, pp_indent, exec_eval);
  }
  
  return ERR_OK;
} /* print_tree */

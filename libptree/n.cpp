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
#include "process.h"

#include "md5f.h"
#include "free.h"
#include "date.h"
#include "constants.h"
#include "i18.h"
#include "timeout.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "s2.h"			/* opts (s2 options) */
#include "str.h"
#include "io.h"                 /* file_ropen(), ... */

#include <sys/time.h>		/* gettimeofday() */
#include <time.h>		/* gettimeofday() */
#include <errno.h>		/* errno */
#include <signal.h>             /* signal() */
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
  EVAL = opts.s2_eval;
  TIMEOUT = opts.s2_timeout;

  /* PCRE options */
  match_opt.px = MATCH_PX;
  match_opt.pcre = MATCH_PCRE;
  
  /* debugging information */
  row = 0;

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
  EVAL = node.EVAL;
  TIMEOUT = node.TIMEOUT;

  /* PCRE options */
  match_opt.px = node.match_opt.px;
  match_opt.pcre = node.match_opt.pcre;

  /* debugging information */
  row = node.row;

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
Node::nodeToString(Node *n, uint indent, Process *proc)
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
  switch(n->COND) {
    case S2_COND_NONE: break;
    case S2_COND_OR: ss << "|| "; break;
    case S2_COND_AND: ss << "&& "; break;
  }

  if(n->REPEAT.type != S2_REPEAT_NONE) {
    ss << '>' << n->REPEAT.X;
    switch(n->REPEAT.type) {
      case S2_REPEAT_NONE: DM_ERR_ASSERT(_("S2_REPEAT_NONE\n")); break;
      case S2_REPEAT_OR: ss << "||"; break;
      case S2_REPEAT_AND: ss << "&&"; break;
      case S2_REPEAT_PAR: ss << ' '; break;
    }
    ss << n->REPEAT.Y << ' ';
  }

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
  if(show_executed && !proc)
    /* don't show node which hasn't executed */
    return ERR_OK;

//  S_P(&thread.print_mtx);
  if(proc && show_executed) {
    if(show_evaluated)
      fprintf(file, "%d/%d: ", proc->executed, proc->evaluated);
    else
      fprintf(file, "%d: ", proc->executed);
  }

  fputs(nodeToString(n, indent, proc).c_str(), file);
  fputc('\n', file);
//  S_V(&thread.print_mtx);

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

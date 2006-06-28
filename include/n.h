#ifndef _NODE_H
#define _NODE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "sysdep.h"             /* BOOL */
#include "match.h"              /* match_opts */

#include "constants.h"

#include <iostream>             /* std::string, cout, endl, ... */
#include <fstream>              /* ifstream */
#include <map>                  /* std::map */
#include <vector>               /* std::vector */

/* constants */
#define THREAD_STACK_SIZE	32768		/* Minimum stack for threads */

/* simple macros */
#define FBRANCH "branch %u (%d/%d): "

#define SS_DQ(space,str)\
  if(str) ss << space << dq_param(Process::eval_str(str,proc), quote)

#define SS_P_DQ(param)\
  if(param) ss << " "#param << "=" << dq_param(Process::eval_str(param,proc), quote)

#define SS_P_DQ_C(c,p,v)\
  if(c) {\
    ss << " "#p << "=" << dq_param(v, quote);\
  } else {\
    DM_LOG(DM_N(1), "no "#c "\n");\
  }

#define SS_P_VALUE(r,param)\
  if(r->param) ss << " "#param << "=" << r->param->value;

#define SS_VEC(vec)\
  if(vec.size()) {\
    ss << " "#vec << "[";\
    DM_DBG(DM_N(4), ""# vec ".size()=%u\n", vec.size());\
    for(uint __u = 0; __u < vec.size(); __u++) {\
      if(__u) ss << " ";\
      ss << dq_param(Process::eval_str(vec[__u],proc), quote);\
    }\
    ss << "]";\
  }\

#define SS_VEC_DEL(vec)\
  do {SS_VEC(vec); if(proc) DELETE_VEC(vec)} while(0)

#define SS_VEC_SPACE\
  do { if(space || print_space) ss << ' ' ; print_space = TRUE;} while(0)

#define SS_P_VEC_PAR(param)\
  if(v[u]) {SS_VEC_SPACE; ss << ""#param << u << "=" << v[u]->param;}

#define SS_P_VEC_DPAR(param)\
  if(v[u] && v[u]->param) {SS_VEC_SPACE; ss << ""#param << u << "=" << *(v[u]->param);}

#define SS_P_VEC_VAL(param)\
  if(v[u]) {SS_VEC_SPACE; ss << ""#param << u << "=" << v[u]->value;}

#define SS_P_VEC_PAR_VAL(param)\
  if(v[u] && v[u]->param) { SS_VEC_SPACE; ss << ""#param << u << "=" << v[u]->param->value; }

typedef enum NODE_t {
  N_GENERAL = 0,		/* node of a general type */
  N_DEFUN = 1,			/* DEFUN process */
  N_FUN = 2,			/* FUN process */
} NODE_t;

struct Node
{
  /* TYPE */
  NODE_t TYPE;			/* Type of the node.  Do not evaluate any children if DEFUN, ... */

  /* OFFSET */
  uint OFFSET;			/* Note: This OFFSET is for parsing purposes only;  *
                                 * any structuring assumptions must be based on     *
                                 * 'child' and 'par' pointers.                      */

  /* COND */
  S2_condition COND;            /* '||', '&&' or nothing */

  /* REPEAT */
  struct {
    S2_repeat type;		/* Repeat type (S2_REPEAT_NONE if normal branch). */
    std::string *X;		/* Repeat from value (string value from the parser). */
    std::string *Y;		/* Repeat to value (string value from the parser). */
  } REPEAT;

  /* EVAL */
  int EVAL;			/* Maximum return value of <ACTION> to evaluate node child. */

  /* TIMEOUT */
  uint64_t TIMEOUT;             /* timeout in micro seconds */

  match_opts match_opt;         /* PCRE options */
#define MATCH_PX        0
#define MATCH_PCRE      PCRE_ANCHORED

  /* debugging/logging information (executed/evaluated _only_ for e2) */
  uint row;			/* parser row (node ID) */
  int executed;			/* execution value of the process (children exclusive) */
  int evaluated;		/* ``complete'' evaluation value of the process (children inclusive); ${!} */

  /* tree overhead */
  Node *parent;			/* the parent of this node; NULL if root */
  Node *child;                  /* a child of this node; NULL if none */
  Node *par;                    /* a following node in parallel to this one; NULL if none */
  Node *rpar;			/* a previous node in parallel to this one; NULL if none */

public:
  Node();
  virtual ~Node();

  virtual void init();
  virtual void init(Node &node);

  static Node *get_node_with_offset(Node *ptr_node, const uint offset);
  static int append_node(Node *root_node, Node *n);
  static std::string nodeToString(Node *n, uint indent, Process *proc);
  static int print_node(Node *n, uint indent, FILE *file, Process *proc, BOOL show_executed, BOOL show_evaluated);

  int print_node(uint indent, FILE *file, Process *proc, BOOL show_executed, BOOL show_evaluated);
  int print_tree(uint indent, FILE *file, BOOL pp_indent, BOOL exec_eval);

  virtual int exec(Process *proc) { return ERR_OK; };
  virtual void finish(Process *proc) { };
  virtual std::string toString(Process *proc) { return "Node"; };

};

#include "process.h"	/* don't move to the top! */

/* derived classes */
struct nAssign : public Node
{
  std::string *overwrite;
  std::vector <std::string *> var;
  std::vector <std::string *> val;

public:
  nAssign();
  nAssign(Node &node);
  ~nAssign();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nDelay : public Node
{
  std::string *sec;
  std::string *nsec;

public:
  nDelay();
  nDelay(Node &node);
  ~nDelay();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nDefun : public Node
{
  std::string *name;				/* function name */
  std::vector <std::string *> params;		/* vector of call by value parameters */
  std::vector <std::string *> params_ref;	/* vector of parameters passed by reference */

public:
  nDefun();
  nDefun(Node &node);
  ~nDefun();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nFun : public Node
{
  std::string *name;			/* function name */
  std::vector <std::string *> args;	/* vector of call by value arguments */
  std::vector <std::string *> args_ref;	/* vector of arguments passed by reference */
  nDefun *nDefunNode;			/* NULL if function not defined */

public:
  nFun();
  nFun(Node &node);
  ~nFun();

  virtual void init();
  int exec(Process *proc);
  int exec(Process *proc, Process &proc_fun);
  void exec_finish(Process *proc, Process &proc_fun);
  std::string toString(Process *proc);
  std::string getByRefVals(Process *proc);

private:

};

struct nNop : public Node
{
  std::string *val;

public:
  nNop();
  nNop(Node &node);
  ~nNop();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nSetenv : public Node
{
  std::string *overwrite;
  std::vector <std::string *> var;
  std::vector <std::string *> val;

public:
  nSetenv();
  nSetenv(Node &node);
  ~nSetenv();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nSystem : public Node
{
  std::string *out;
  std::string *cmd;

public:
  nSystem();
  nSystem(Node &node);
  ~nSystem();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

struct nTest : public Node
{
  std::string *expr;

public:
  nTest();
  nTest(Node &node);
  ~nTest();

  virtual void init();
  int exec(Process *proc);
  std::string toString(Process *proc);

private:

};

/* external variables (defined by other modules) */
extern struct opts_t opts;		/* command-line options */

#endif /* _NODE_H */

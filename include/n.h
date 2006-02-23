#ifndef _NODE_H
#define _NODE_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "sysdep.h"             /* BOOL */
#include "match.h"              /* match_opts */

#include "constants.h"
#include "var_table.h"

#include <iostream>             /* std::string, cout, endl, ... */
#include <fstream>              /* ifstream */
#include <map>                  /* std::map */
#include <vector>               /* std::vector */
#include <pthread.h>            /* `parallel' execution of branches */

/* constants */
#define THREAD_STACK_SIZE	32768		/* Minimum stack for threads */

/* simple macros */
#define SS_DQ(space,str)\
  if(str) ss << space << dq_param(eval_str(str, eval), quote)

#define SS_P_DQ(param)\
  if(param) ss << " "#param << "=" << dq_param(eval_str(param, eval), quote)

#define SS_P_VALUE(r,param)\
  if(r->param) ss << " "#param << "=" << r->param->value;

#define SS_VEC(vec)\
  if(vec.size()) {\
    ss << " "#vec << "[";\
    for(uint i = 0; i < vec.size(); i++) {\
      if(i) ss << "][";\
      ss << Node::ind_param(vec[i]);\
    }\
    ss << "]";\
  }

#define SS_VEC_DEL(vec)\
  do {SS_VEC(vec); if(eval) DELETE_VEC(vec)} while(0)

#define SS_VEC_SPACE\
  do { if(space || print_space) ss << ' ' ; print_space = TRUE;} while(0)

#define SS_P_VEC_PAR(param)\
  if(v[i]) {SS_VEC_SPACE; ss << ""#param << i << "=" << v[i]->param;}

#define SS_P_VEC_VAL(param)\
  if(v[i]) {SS_VEC_SPACE; ss << ""#param << i << "=" << v[i]->value;}

#define SS_P_VEC_PAR_VAL(param)\
  if(v[i] && v[i]->param) { SS_VEC_SPACE; ss << ""#param << i << "=" << v[i]->param->value; }

#define _TYPEDEF_PINT(u,s)\
typedef struct p##u##int##s##_t \
{\
  u##int##s##_t *p;\
  u##int##s##_t v;\
} p##u##int##s##_t;
_TYPEDEF_PINT(,32);
_TYPEDEF_PINT(,64);
_TYPEDEF_PINT(u,32);
_TYPEDEF_PINT(u,64);
#undef _TYPEDEF_PINT

typedef enum CMP_t {
  CMP_EQ = 0,			/* == */
  CMP_NE = 1,			/* != */
  CMP_LT = 2,			/* < */
  CMP_GT = 3,			/* > */
  CMP_LE = 4,			/* <= */
  CMP_GE = 5,			/* >= */
} CMP_t;

struct Node
{
  /* OFFSET */
  uint OFFSET;			/* Note: This OFFSET is for parsing purposes only;  *
                                 * any structuring assumptions must be based on     *
                                 * 'child' and 'par' pointers.                      */

  /* COND */
  S2_condition COND;            /* '||', '&&' or nothing */

  /* REPEAT */
  struct {
    S2_repeat type;             /* Repeat type (S2_REPEAT_NONE if normal branch). */
    int64_t X;                  /* Repeat from value when !S2_REPEAT_NONE, 0 otherwise. */
    int64_t Y;                  /* Repeat to value when !S2_REPEAT_NONE, 0 otherwise. */
    int64_t I;			/* Counter value when !(S2_REPEAT_NONE && S2_REPEAT_PAR), 0 otherwise. */
  } REPEAT;

  /* EVAL */
  int EVAL;			/* Maximum return value of <ACTION> to evaluate node child. */

  /* TIMEOUT */
  uint64_t TIMEOUT;             /* timeout in micro seconds */

  match_opts match_opt;         /* PCRE options */
#define MATCH_PX        0
#define MATCH_PCRE      PCRE_ANCHORED

  /* debugging information */
  uint row;			/* parser row (node ID) */
  BOOL has_executed;		/* has this node been executed at all? */
  int executed;			/* execution value of the node (children exclusive) */
  int evaluated;		/* ``complete'' evaluation value of the node (children inclusive) */

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
  static int threads_init(void);
  static int threads_destroy(void);

  static Node *get_node_with_offset(Node *ptr_node, const uint offset);
  static int append_node(Node *root_node, Node *n);
  static std::string dq_param(const std::string *s, BOOL quote);
  static std::string dq_param(const std::string &s, BOOL quote);
  static std::string dq_param(const char *s, BOOL quote);
  static std::string dq_param(const bool b, BOOL quote);
  static std::string dq_param(const unsigned char c, BOOL quote);
  static std::string ind_param(const std::string *s);
  static std::string ind_param(const std::string &s);
  static std::string ind_param(const char *s);
  std::string nodeToString(uint indent, BOOL eval);
  int print_node(uint indent, FILE *file, BOOL eval, BOOL show_executed, BOOL show_evaluated);
  int print_tree(uint indent, FILE *file, BOOL pp_indent, BOOL eval, BOOL exec_eval);
  int eval();
  int eval_repeats();
  int eval_sequential_repeats();
  int eval_with_timeout();
  int eval_without_timeout();
  int eval_subtree(const int root_exec, int &root_eval);
  BOOL e_match(const std::string *expected, const char *received);
  BOOL e_match(const char *expected, const char *received);

  virtual int exec() { return ERR_OK; };
  virtual std::string toString(BOOL eval) { return "Node"; };

protected:
  /* evaluation functions (basic) */
  std::string eval_str(const char *cstr, BOOL eval);
  std::string eval_str(const std::string *s, BOOL eval);
#define EVAL2CSTR(str) (str)? eval_str(str, TRUE).c_str(): (const char *)NULL

#define _EVAL2INT(u,s) u##int##s##_t eval2##u##int##s(const std::string *str)
  _EVAL2INT(,32);
  _EVAL2INT(,64);
  _EVAL2INT(u,32);
  _EVAL2INT(u,64);
#undef _EVAL2INT
#define _EVAL2PINT(u,s) p##u##int##s##_t eval2p##u##int##s(const std::string *str)
  _EVAL2PINT(,32);
  _EVAL2PINT(,64);
  _EVAL2PINT(u,32);
  _EVAL2PINT(u,64);
#undef _EVAL2PINT

  /* evaluation functions (vectors) */
  std::vector <std::string *> eval_vec_str(const std::vector <std::string *> &v, BOOL eval);
#define EVAL_VEC_STR(r,vec) vec = eval? eval_vec_str((r::vec), TRUE): (r::vec)

#define _EVAL_VEC_INT(u,s)\
  std::vector <u##int##s##_t> \
  Node::eval_vec_##u##int##s(const std::vector <std::string *> &v, BOOL eval)
  _EVAL_VEC_INT(,32);
  _EVAL_VEC_INT(,64);
  _EVAL_VEC_INT(u,32);
  _EVAL_VEC_INT(u,64);
#undef _EVAL_VEC_INT

#define _EVAL_VEC_PINT(u,s)\
  std::vector <u##int##s##_t *> \
  Node::eval_vec_p##u##int##s(const std::vector <std::string *> &v, BOOL eval)
  _EVAL_VEC_PINT(,32);
  _EVAL_VEC_PINT(,64);
  _EVAL_VEC_PINT(u,32);
  _EVAL_VEC_PINT(u,64);
#undef _EVAL_VEC_PINT

};

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
  int exec();
  std::string toString(BOOL eval);

private:

};

struct nCmp : public Node
{
  CMP_t cmp;
  std::string *lop;
  std::string *rop;

public:
  nCmp();
  nCmp(Node &node);
  ~nCmp();

  virtual void init();
  int exec();
  std::string toString(BOOL eval);

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
  int exec();
  std::string toString(BOOL eval);

private:

};

struct nMatch : public Node
{
  std::string *expected;
  std::string *received;

public:
  nMatch();
  nMatch(Node &node);
  ~nMatch();

  virtual void init();
  int exec();
  std::string toString(BOOL eval);

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
  int exec();
  std::string toString(BOOL eval);

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
  int exec();
  std::string toString(BOOL eval);

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
  int exec();
  std::string toString(BOOL eval);

private:

};


/* external variables (defined by other modules) */
extern struct opts_t opts;		/* command-line options */

#endif /* _NODE_H */

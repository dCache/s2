#ifndef _PROCESS_H
#define _PROCESS_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "n.h"			/* Node */

#include <pthread.h>            /* `parallel' execution of branches */
#include <sys/time.h>		/* timespec */
#include <time.h>		/* timespec */

#include <map>                  /* std::map */

/* typedefs */
typedef std::map<std::string, std::string> TVariables;		/* name/value pair */

/* global variables */
static TVariables gl_var_tab;	/* table of global variables */

struct Process
{
  /* FUNCTION */
  BOOL fun;			/* Is this a function call? */

  /* OFFSET */
  uint OFFSET;			/* Additional OFFSET/indentation for function calls */

  Node *n;			/* pointer to the Node that's being evaluated */

  /* REPEAT */
  int64_t I;			/* Counter value when !(S2_REPEAT_NONE && S2_REPEAT_PAR), X otherwise. */

  /* debugging information */
  int executed;			/* execution value of the process (children exclusive) */
  int evaluated;		/* ``complete'' evaluation value of the process (children inclusive); ${!} */

  /* tree overhead (${?}, ${!}) */
  Process *parent;		/* the parent of this process; NULL if root */
  Process *rpar;		/* a process which finished execution before this one (same indentation); NULL if none */

  TVariables *var_tab;		/* local variables (if it is a process running in parallel) */

public:
  Process();
  Process(Node *node, Process *p, Process *rpar);
  virtual ~Process();
  static int threads_init(void);
  static int threads_destroy(void);

  virtual void init();
  virtual void init(Node *node, Process *p, Process *rpar);

  int eval();
  int eval_par();
  int eval_repeats();
  int eval_sequential_repeats();
  int exec_with_timeout();
  int eval_with_timeout();
  int eval_subtree(const int root_exec, int &root_eval);

  BOOL is_parallel();

  BOOL e_match(const std::string *expected, const char *received);
  BOOL e_match(const char *expected, const char *received);

  /* operations on variables */
  void WriteVariable(TVariables *var_tab, const char *name, const char *value, int vlen);
  void WriteVariable(Process *proc, const char *name, const char *value, int vlen);
  void WriteVariable(const char *name, const char *value);
  void WriteVariable(const char *name, const char *value, int vlen);
  const char *ReadVariable(TVariables *var_tab, const char *name);
  const char *ReadVariable(Process *proc, const char *name);
  const char *ReadVariable(const char *name);

  std::string toString();

  /* evaluation functions (basic) */
  static std::string eval_str(const std::string *s, Process *proc);
  static std::string eval_str(const char *cstr, Process *proc);
#define EVAL2CSTR(str) (str)? Process::eval_str(str, proc).c_str(): (const char *)NULL

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
  std::vector <std::string *> eval_vec_str(const std::vector <std::string *> &v);
#define EVAL_VEC_STR(r,vec) vec = proc? proc->eval_vec_str((r::vec)): (r::vec)

#define _EVAL_VEC_INT(u,s)\
  std::vector <u##int##s##_t> \
  eval_vec_##u##int##s(const std::vector <std::string *> &v)
  _EVAL_VEC_INT(,32);
  _EVAL_VEC_INT(,64);
  _EVAL_VEC_INT(u,32);
  _EVAL_VEC_INT(u,64);
#undef _EVAL_VEC_INT

#define _EVAL_VEC_PINT(u,s)\
  std::vector <u##int##s##_t *> \
  eval_vec_p##u##int##s(const std::vector <std::string *> &v)
  _EVAL_VEC_PINT(,32);
  _EVAL_VEC_PINT(,64);
  _EVAL_VEC_PINT(u,32);
  _EVAL_VEC_PINT(u,64);
#undef _EVAL_VEC_PINT

};


#endif /* _PROCESS_H */

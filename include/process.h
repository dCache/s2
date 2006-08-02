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

/* simple macros */
#define S_P(mtx,proc)\
  do {\
    pthread_mutex_lock(mtx);\
    Mutexes_t::iterator iter;\
    if((iter = mutex_tab.find(mtx)) != mutex_tab.end()) {\
      iter->second = proc;\
    } else {\
      mutex_tab.insert(std::pair<pthread_mutex_t *, Process *>(mtx, proc));\
    }\
  } while(0)
#define S_V(mtx)\
  do {\
    pthread_mutex_unlock(mtx);\
  } while(0)
#define MUTEX(mtx,proc,...)\
  do {S_P(mtx,proc);\
      DM_DBG(DM_N(3), "<<< mutex "# mtx"\n");\
      __VA_ARGS__ ;\
      DM_DBG(DM_N(3), "mutex "# mtx" >>>\n");\
      S_V(mtx);} while(0)

/* typedefs */
typedef std::map<std::string, std::string> Vars_t;		/* name/value pair */
typedef std::map<pthread_mutex_t *, Process *> Mutexes_t;

#define _TYPEDEF_PINT(u,s)\
typedef struct p##u##int##s##_t \
{\
  u##int##s##_t *p;\
  u##int##s##_t v;\
} p##u##int##s##_t;
_TYPEDEF_PINT(,);
_TYPEDEF_PINT(,32);
_TYPEDEF_PINT(,64);
_TYPEDEF_PINT(u,32);
_TYPEDEF_PINT(u,64);
#undef _TYPEDEF_PINT


typedef enum EVAL_t {
  EVAL_NONE = 0,		/* do not evaluate anything */
  EVAL_ALL = 1,			/* evaluate all variables and tags */
  EVAL_STATIC = 2,		/* evaluate only variables and `static' tags (e.g. not $DATE{}, $RND, ... */
} EVAL_t;


/* global variables */
static Vars_t gl_var_tab;	/* table of global variables */
static Mutexes_t mutex_tab;	/* table of locked mutexes */

struct Process
{
  /* FUNCTION */
  BOOL fun;			/* Is this a function call? */

  /* FUN_OFFSET */
  uint FUN_OFFSET;		/* Special offset for function calls */

  Node *n;			/* pointer to the Node that's being evaluated */

  /* REPEAT */
  int64_t I;			/* Counter value when !(S2_REPEAT_NONE && S2_REPEAT_PAR), X otherwise. */

  /* execution/evaluation values for the internal S2 logic */
  EVAL_t et;			/* evaluation type */
  int executed;			/* execution value of the process (children exclusive) */
  int evaluated;		/* ``complete'' evaluation value of the process (children inclusive); ${!} */
  void *ret;			/* pointer to data returned by executing this process */

  /* tree overhead (${?}, ${!}) */
  Process *parent;		/* the parent of this process; NULL if root */
  Process *rpar;		/* a process which finished execution before this one (same indentation); NULL if none */

  Vars_t *var_tab;		/* local variables (if it is a process running in parallel) */

public:
  Process();
  Process(Node *node, Process *p, Process *rpar);
  virtual ~Process();
  static void progress(int show, Process *proc);
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
  int eval_subtree(const int root_exec);

  BOOL is_parallel();

  BOOL e_match(const std::string *expected, const char *received);
  BOOL e_match(const char *expected, const char *received);

  /* operations on variables */
  void WriteVariable(Vars_t *var_tab, const char *name, const char *value, int vlen);
  void WriteVariable(Process *proc, const char *name, const char *value, int vlen);
  void WriteVariable(const char *name, const char *value, BOOL argv);
  void WriteVariable(const char *name, const char *value, BOOL argv, int vlen);
  const char *ReadVariable(Vars_t *var_tab, const char *name);
  const char *ReadVariable(Process *proc, const char *name);
  const char *ReadVariable(const char *name);

  std::string toString();

  /* evaluation functions (basic) */
  static std::string eval_str(const char *cstr, Process *proc);
  static std::string eval_str(const std::string *s, Process *proc);
#define EVAL2CSTR(str) (str)? Process::eval_str(str, proc).c_str(): (const char *)NULL

#define _EVAL2INT(u,s) u##int##s##_t eval2##u##int##s(const std::string *str)
  _EVAL2INT(,);
  _EVAL2INT(,32);
  _EVAL2INT(,64);
  _EVAL2INT(u,32);
  _EVAL2INT(u,64);
#undef _EVAL2INT
#define _EVAL2PINT(u,s) p##u##int##s##_t eval2p##u##int##s(const std::string *str)
  _EVAL2PINT(,);
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
  _EVAL_VEC_INT(,);
  _EVAL_VEC_INT(,32);
  _EVAL_VEC_INT(,64);
  _EVAL_VEC_INT(u,32);
  _EVAL_VEC_INT(u,64);
#undef _EVAL_VEC_INT

#define _EVAL_VEC_PINT(u,s)\
  std::vector <u##int##s##_t *> \
  eval_vec_p##u##int##s(const std::vector <std::string *> &v)
  _EVAL_VEC_PINT(,);
  _EVAL_VEC_PINT(,32);
  _EVAL_VEC_PINT(,64);
  _EVAL_VEC_PINT(u,32);
  _EVAL_VEC_PINT(u,64);
#undef _EVAL_VEC_PINT

private:
  std::string preeval_str(const std::string *s);
  std::string preeval_str(const char *cstr);

};

/* global variables */
extern struct tp_sync_t tp_sync;

#endif /* _PROCESS_H */

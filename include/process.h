#ifndef _PROCESS_H
#define _PROCESS_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include <pthread.h>            /* `parallel' execution of branches */
#include <sys/time.h>		/* timespec */
#include <time.h>		/* timespec */

struct Process
{
  Node *n;			/* pointer to the Node that's being evaluated */

  /* REPEAT */
  int64_t I;			/* Counter value when !(S2_REPEAT_NONE && S2_REPEAT_PAR), X otherwise. */

  /* vector of parallel processes spawned by this process */
  std::vector <Process *> vProc;

  /* tree overhead */
  Process *par;			/* a following process in parallel to this one; NULL if none */
  Process *rpar;		/* a previous process in parallel to this one; NULL if none */

public:
  Process();
  Process(Node *node, Process *p);
  virtual ~Process();
  static int threads_init(void);
  static int threads_destroy(void);

  virtual void init();
  virtual void init(Node *node, Process *p);

  int eval();
  int eval_repeats();
  int eval_sequential_repeats();
  int exec_with_timeout();
  int eval_with_timeout();
  int eval_subtree(const int root_exec, int &root_eval);

  std::string toString(BOOL eval);

};


#endif /* _PROCESS_H */

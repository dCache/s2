#ifndef _PROCESS_H
#define _PROCESS_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include <pthread.h>            /* `parallel' execution of branches */

struct Process
{
  Node *n;			/* pointer to the Node that's being evaluated */

  /* REPEAT */
  int64_t I;			/* Counter value when !(S2_REPEAT_NONE && S2_REPEAT_PAR), X otherwise. */

  /* vector of parallel processes spawned by this process */
  std::vector <Process *> vProc;

public:
  Process();
  Process(Node *node);
  virtual ~Process();
  static int threads_init(void);
  static int threads_destroy(void);

  virtual void init();
  virtual void init(Node *node);

  int eval();
  int eval_repeats();
  int eval_sequential_repeats();
  int eval_with_timeout();
  int eval_without_timeout();
  int eval_subtree(const int root_exec, int &root_eval);

  std::string toString(BOOL eval);

};


#endif /* _PROCESS_H */

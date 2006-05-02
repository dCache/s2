#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#ifdef HAVE_INTTYPES
#include <stdint.h>
#endif

#include "sysdep.h"             /* BOOL */

#include "constants.h"

#include <sys/types.h>          /* timeval */
#include <pthread.h>            /* pthread_t */

/* Simple macros */
#ifndef RETURN
#define RETURN(...) do {DM_DBG_O; return __VA_ARGS__;} while(0)
#endif

#define S_P(mtx)	pthread_mutex_lock(mtx)
#define S_V(mtx)	pthread_mutex_unlock(mtx)
#define MUTEX(mtx,...)\
  do {S_P(mtx); __VA_ARGS__ ; S_V(mtx);} while(0)

#define TP_THREADS_MAX	128		/* maximum number of threads in the threadpool */
#define TP_THREADS_MIN	1		/* minimum number of threads in the threadpool */
#define TP_THREADS_DEF	8		/* default number of threads in the threadpool */

/* format of a single request. */
typedef struct tp_request {
  void *data;				/* request's data */
  int tp_tid;				/* number of the thread handling this request */
  int *sreqs;				/* pointer to the total number of parallel requests *
                                         * connected with this request                      */
  pthread_cond_t *p_sreqs_cv;		/* pointer to the conditional variable */
  struct tp_request *next;		/* pointer to next request, NULL if none */
} tp_request;

/* extern(al) function declarations */
extern int tp_init(int tp_size);
extern int tp_cleanup(void);
extern int tp_enqueue(void *data, int *sreqs, pthread_cond_t *p_sreqs_cv);
extern int tp_dequeue(void *data);

/* extern(al) function declarations (defined in other modules) */
extern void *tp_handle_request(void *request);

#endif /* _THREAD_POOL_H */

#ifndef _TIMEOUT_H
#define _TIMEOUT_H

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

/* extern(al) function declarations */
extern void timeout_add(pthread_t tid, uint64_t usec);
extern void timeout_del(pthread_t tid);
extern void timeout_sched(void);
extern void timeout_print(void);

#endif /* _TIMEOUT_H */

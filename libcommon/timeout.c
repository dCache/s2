#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "timeout.h"

#include "free.h"
#include "constants.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include "str.h"
#include "io.h"                 /* file_ropen(), ... */

#include <errno.h>
#include <limits.h>             /* PTHREAD_THREADS_MAX */
#include <signal.h>             /* signal() */
#include <stdio.h>              /* stderr */
#include <stdlib.h>             /* exit() */
#include <sys/time.h>
#include <time.h>
#include <unistd.h>             /* usleep() */

#include <string>               /* std::string */
#include <vector>               /* std::vector */
#include <iostream>             /* std::string, cout, endl, ... */

using namespace std;

/* local variables */
pthread_attr_t thread_attr;
typedef std::pair<pthread_t, timeval> ptt;
std::vector<ptt> vTimeouts;

/* private functions */
static void timer_off(void);
static void timer_set(timeval timeout);

/*
 * Signal handler.
 */
static void
sig_alrm(int sig)
{
  DM_DBG_I;

  timer_off();
  DM_DBG(DM_N(1), "caught SIGALARM\n");
  timeout_sched();

  DM_DBG_O;
}

/*
 * Set SIGALRM timeout.
 */
static void
timer_set(timeval timeout)
{
  DM_DBG_I;

  struct sigaction act;
  struct itimerval timer;

  act.sa_handler = sig_alrm;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;           /* don't fiddle with EINTR */
  sigaction(SIGALRM, &act, NULL);
  
  /* next value (set to the reset value) */
  timer.it_interval.tv_sec = timeout.tv_sec;
  timer.it_interval.tv_usec = timeout.tv_usec;
  /* current value (set to the amount of time remaining on the timer) */
  timer.it_value.tv_sec = timeout.tv_sec;
  timer.it_value.tv_usec = timeout.tv_usec;

  if(setitimer(ITIMER_REAL, &timer, NULL) != 0)
  {
    DM_ERR(ERR_SYSTEM, _("setitimer: %s\n"), strerror(errno));
    RETURN();
  }

  DM_DBG_O;
}

/*
 * Turn timer off.
 */
static void
timer_off(void)
{
  DM_DBG_I;

  struct timeval timestamp;

  timestamp.tv_sec = 0;
  timestamp.tv_usec = 0;
  timer_set(timestamp);         /* ALARM */

  DM_DBG_O;
}

/*
 * Insert thread's id in the sorted timeout list.
 */
extern void
timeout_add(pthread_t tid, uint64_t usec)
{
  DM_DBG_I;

  struct timeval timestamp, timeout;

  timer_off();

  timeout.tv_sec = usec / 1000000;
  timeout.tv_usec = usec - (timeout.tv_sec * 1000000);

  if(gettimeofday(&timestamp, NULL)) {
    DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
    RETURN();
  };
  timestamp.tv_sec += timeout.tv_sec;
  timestamp.tv_usec += timeout.tv_usec;

  bool inserted = false;
  std::vector<ptt>::iterator vi = vTimeouts.begin();
  for (; vi != vTimeouts.end(); vi++) {
    if( timestamp.tv_sec <  (*vi).second.tv_sec ||
       (timestamp.tv_sec == (*vi).second.tv_sec && timestamp.tv_usec < (*vi).second.tv_usec)) {
      DM_DBG(DM_N(3), "inserting %lu; tv_sec/tv_usec (%ld/%ld)\n", tid, timestamp.tv_sec, timestamp.tv_usec);
      vTimeouts.insert(vi, std::pair<pthread_t, timeval>(tid, timestamp));
      inserted = true;
      break;
    }
  }
  if(!inserted) {
    DM_DBG(DM_N(3), "inserting %lu; tv_sec/tv_usec (%ld/%ld)\n", tid, timestamp.tv_sec, timestamp.tv_usec);
    vTimeouts.push_back(std::pair<pthread_t, timeval>(tid, timestamp));
  }

  timeout_sched();
  DM_DBG_O;
}

/*
 * Thread terminated normally => remove thread's record from the timeout list.
 */
extern void
timeout_del(pthread_t tid)
{
  DM_DBG_I;

  std::vector<ptt>::iterator vi = vTimeouts.begin();

  timer_off();

  for (; vTimeouts.size() != 0 && vi != vTimeouts.end(); vi++) {
    if(tid == (*vi).first) {
      DM_DBG(DM_N(2), "deleting %lu\n", tid);
      vTimeouts.erase(vi);
      break;
    }
  }

  timeout_sched();

  DM_DBG_O;
}

/*
 * Thread timeout scheduler.  Maintains the timeout list.
 */
extern void
timeout_sched(void)
{
  DM_DBG_I;

  struct timeval timestamp;

  std::vector<ptt>::iterator vi = vTimeouts.begin();
  if(vTimeouts.size() == 0 || vi == vTimeouts.end()) RETURN();

  if(gettimeofday(&timestamp, NULL)) {
    DM_ERR(ERR_SYSTEM, _("gettimeofday: %s\n"), strerror(errno));
    RETURN();
  };

  timestamp.tv_sec = (*vi).second.tv_sec - timestamp.tv_sec;
  timestamp.tv_usec = (*vi).second.tv_usec - timestamp.tv_usec;
  while (timestamp.tv_usec < 0) {
    timestamp.tv_sec -= 1;
    timestamp.tv_usec += 1000000L;
  }
  DM_DBG(DM_N(2), "timeout_sched: (%lu/%ld/%ld)\n", (*vi).first, timestamp.tv_sec, timestamp.tv_usec);

  if(timestamp.tv_sec > 0 || 
    (timestamp.tv_sec == 0 && timestamp.tv_usec > 0)) {
    DM_DBG(DM_N(2), "setting timer: (%lu/%ld/%ld)\n", (*vi).first, timestamp.tv_sec, timestamp.tv_usec);
    timer_set(timestamp);       /* ALARM */
  } else {
    /* timeout in the past, cancel thread and remove it from the timeout list */
    pthread_t thr_cancel = (*vi).first;
//    timeout_print();
    DM_DBG(DM_N(2), "timeout in the past, cancelling thread %lu\n", thr_cancel);
    vTimeouts.erase(vi);
//    timeout_print();
    timeout_sched();
    pthread_cancel(thr_cancel);
  }

  DM_DBG_O;
}
extern void
timeout_print(void)
{
  std::vector<ptt>::iterator vi = vTimeouts.begin();
  for (; vi != vTimeouts.end(); vi++) {
    DM_DBG(DM_N(2), "(%lu/%ld/%ld) ", (*vi).first, (*vi).second.tv_sec, (*vi).second.tv_usec);
  }
  DM_DBG(DM_N(2), "\n");
}

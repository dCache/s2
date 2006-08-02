#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_INTTYPES
#include "inttypes.h"
#endif

#ifdef DG_DIAGNOSE
#include "diagnose/dg.h"
#endif

#include "thread_pool.h"

#include "free.h"
#include "i18.h"
#include "sysdep.h"             /* BOOL, STD_BUF, ... */

#include <errno.h>		/* errno */
#include <stdio.h>		/* standard I/O routines */
#include <pthread.h>		/* pthread functions and data structures */
#include <string.h>		/* strerror() */
#include <stdlib.h>		/* rand() and srand() functions */
#include <unistd.h>

/* simple macros */
#define S_P(mtx)	pthread_mutex_lock(mtx);
#define S_V(mtx)	pthread_mutex_unlock(mtx);
#define MUTEX(mtx,...)\
  do {S_P(mtx);\
      DM_DBG(DM_N(3), "<<< mutex "# mtx"\n");\
      __VA_ARGS__ ;\
      DM_DBG(DM_N(3), "mutex "# mtx" >>>\n");\
      S_V(mtx);} while(0)

/* number of threads used to service requests */
#define FTHREAD	"thread (%d/%lu): "

/* Global mutex for our program.  Assignment initializes it. */
/* Note that we use a RECURSIVE mutex, since a handler       */
/* thread might try to lock it twice consecutively.          */
static pthread_mutex_t request_mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_mutex_t threads_total_mtx;
static int threads_total;

/* Global condition variable for our program. Assignment initializes it. */
static pthread_cond_t request_cv = PTHREAD_COND_INITIALIZER;

/* Thread's structures */
static pthread_t p_threads[TP_THREADS_MAX];
static int thr_id[TP_THREADS_MAX];			/* thread IDs */
static int tp_size;

static int num_requests = 0;		/* number of pending requests, initially none */
static int done_creating_requests = 0;	/* are we done creating new requests? */

static tp_request *requests = NULL;	/* head of linked list of requests */
static tp_request *last_request = NULL;	/* pointer to last request */

/* Private functions declarations */
static int tp_create(void);

/*
 * function request_add(): add a request to the requests list
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex
 * output:    none
 */
static int
request_add(void *data,
            int *sreqs,
            pthread_mutex_t *p_sreqs_mtx,
            pthread_cond_t *p_sreqs_cv)
{
  DM_DBG_I;

  int rc;				/* return code of pthreads functions */
  tp_request *request;			/* pointer to newly added request */
  
  /* create structure with new request */
  request = (tp_request *) malloc(sizeof(tp_request));
  if (!request) {			/* malloc failed? */
    DM_ERR(ERR_SYSTEM, _("malloc failed: %s\n"), strerror(errno));
    RETURN(ERR_ERR);
  }
  /* fill the request payload */
  request->data = data;
  request->sreqs = sreqs;
  request->p_sreqs_mtx = p_sreqs_mtx;
  request->p_sreqs_cv = p_sreqs_cv;
  request->next = NULL;

  /* lock the mutex, to assure exclusive access to the list */
  rc = S_P(&request_mtx);

  /* add new request to the end of the list, updating list */
  /* pointers as required */
  if (num_requests == 0) {		/* special case - list is empty */
    requests = request;
    last_request = request;
  } else {
    last_request->next = request;
    last_request = request;
  }

  num_requests++;	/* number of pending requests */

  DM_DBG(DM_N(3), "added request %d\n", num_requests);

  /* unlock mutex */
  rc = S_V(&request_mtx);
  if(rc) {
    DM_ERR(ERR_SYSTEM, _("pthread_mutex_unlock failed: %s\n"), strerror(errno));
    RETURN(ERR_ERR);
  }

  /* signal the condition variable - there's a new request to handle */
  rc = pthread_cond_signal(&request_cv);
  if(rc) {
    DM_ERR(ERR_SYSTEM, _("pthread_cond_signal failed: %s\n"), strerror(errno));
    RETURN(ERR_ERR);
  }
  
  RETURN(ERR_OK);  
}

/*
 * function request_del(): delete a request from the requests list
 */
static int
request_del(void *data)
{
  DM_DBG_I;

  int rc;				/* return code of pthreads functions */
  tp_request *request = requests;	/* pointer to a request */
  
  if(requests == NULL) {
    DM_DBG(DM_N(3), "couldn't dequeue a request %p, list of requests is empty\n", data);
    RETURN(ERR_ERR);
  }

  /* lock the mutex, to assure exclusive access to the list */
  rc = S_P(&request_mtx);

  /* check the special case (head) */
  if(requests->data == data) {
    /* found a matching request */
    request = requests->next;
    FREE(requests);
    requests = request;
    num_requests--;			/* number of pending requests */
    DM_DBG(DM_N(3), "dequeued a request %p, total requests %d\n", data, num_requests);
    goto ok;
  }

  /* go through the requests list */
  while(request->next) {
    if(request->next->data == data) {
      /* found a matching request */
      tp_request *tmp_request = request->next->next;
      FREE(request->next);
      request->next = tmp_request;
      num_requests--;			/* number of pending requests */
      DM_DBG(DM_N(3), "dequeued a request %p, total requests %d\n", data, num_requests);
      goto ok;
    }
    request = request->next;
  }
  /* couldn't dequeue a request */
  DM_DBG(DM_N(3), "couldn't dequeue a request %p, total requests %d\n", data, num_requests);

  /* unlock mutex */
  rc = S_V(&request_mtx);

  if(rc) {
    DM_ERR(ERR_SYSTEM, _("pthread_mutex_unlock failed: %s\n"), strerror(errno));
    RETURN(ERR_ERR);
  }

  /* couldn't dequeue a request */
  RETURN(ERR_ERR);

ok:
  /* unlock mutex */
  rc = S_V(&request_mtx);

  if(rc) {
    DM_ERR(ERR_SYSTEM, _("pthread_mutex_unlock failed: %s\n"), strerror(errno));
    RETURN(ERR_ERR);
  }

  RETURN(ERR_OK);
}

/*
 * function request_get(): gets the first pending request from the requests list
 *                         removing it from the list.
 * algorithm: creates a request structure, adds to the list, and
 *            increases number of pending requests by one.
 * input:     request number, linked list mutex
 * output:    pointer to the removed request, or NULL if none
 * memory:    the returned request need to be freed by the caller
 */
static tp_request *
request_get(void)
{
  DM_DBG_I;
  
  int rc;				/* return code of pthreads functions */
  tp_request *request;			/* pointer to request */

  /* lock the mutex, to assure exclusive access to the list */
  rc = S_P(&request_mtx);

  if (num_requests > 0) {
    request = requests;
    requests = request->next;
    if (requests == NULL) {		/* this was the last request on the list */
      last_request = NULL;
    }
    num_requests--;			/* the total number of pending requests */
  } else {				/* requests list is empty */
    request = NULL;
  }

  /* unlock mutex */
  rc = S_V(&request_mtx);

  /* return the request to the caller. */
  RETURN(request);
}

/*
 * function request_loop_handler(): infinite loop of requests handling
 * algorithm: forever, if there are requests to handle, take the first
 *            and handle it. Then wait on the given condition variable,
 *            and when it is signaled, re-do the loop.
 *            Increases number of pending requests by one.
 * input:     id of thread, for printing purposes
 * output:    none
 */
static void *
request_loop_handler(void *p_tp_tid)
{
  DM_DBG_I;

  int rc;				/* return code of pthreads functions */
  tp_request *request;			/* pointer to a request */
#if defined(DG_DBG) && defined(DG_DIAGNOSE)
  pthread_t tid = pthread_self();	/* thread identifying number */
#endif
  int tp_tid;				/* thread-pool related thread's identifying number */
  
  if(p_tp_tid == NULL) {
    DM_ERR_ASSERT("opt == NULL\n");
    RETURN(NULL);
  }
  tp_tid = *((int *) p_tp_tid);

  DM_DBG(DM_N(3), FTHREAD"starting\n", tp_tid, tid);

  /* lock the mutex, to access the requests list exclusively */
  rc = S_P(&request_mtx);

  DM_DBG(DM_N(3), FTHREAD"after pthread_mutex_lock\n", tp_tid, tid);

  /* set my cancel state to 'enabled', and cancel type to 'defered'. */
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

  /* do forever... */
  while (1) {
    DM_DBG(DM_N(3), FTHREAD"requests=%d; threads_total=%d\n", tp_tid, tid, num_requests, threads_total);
    if (num_requests > 0) {		/* a request is pending */
      request = request_get();
      request->tp_tid = tp_tid;

      if (request) {			/* got a request - handle it and free it */
	/* unlock mutex - so other threads would be able to handle */
	/* other requests waiting in the queue paralelly */
	rc = S_V(&request_mtx);

        DM_DBG(DM_N(3), FTHREAD"handling request (%d/%p)\n", tp_tid, tid, request->tp_tid, request);
        tp_handle_request(request);
        DM_DBG(DM_N(3), FTHREAD"handled request (%d/%p)\n", tp_tid, tid, request->tp_tid, request);

	/* and lock the request mutex again */
	rc = S_P(&request_mtx);

        S_P(request->p_sreqs_mtx);
        if(request->sreqs) {
          DM_DBG(DM_N(3), FTHREAD"sreqs=%d\n", tp_tid, tid, *request->sreqs);
          *request->sreqs -= 1;
          DM_DBG(DM_N(3), FTHREAD"sreqs=%d\n", tp_tid, tid, *request->sreqs);

          if(*(request->sreqs) == 0) {
            DM_DBG(DM_N(3), FTHREAD"sreqs=0 => signalling sreqs_cv\n", tp_tid, tid);
            rc = pthread_cond_signal(request->p_sreqs_cv);
            if(rc) {
              DM_ERR(ERR_SYSTEM, _(FTHREAD"pthread_cond_signal failed: %s\n"), tp_tid, tid, strerror(errno));
              RETURN(NULL);
            }
          } else {
            DM_DBG(DM_N(3), FTHREAD"sreqs=%d\n", *(request->sreqs));
          }
        }
        S_V(request->p_sreqs_mtx);

        DM_DBG(DM_N(3), FTHREAD"freeing request (%d/%p)\n", tp_tid, tid, request->tp_tid, request);
	FREE(request);
      }
    } else {
      /* On the condition variable.                          */
      /* If no new requests are going to be generated, exit. */
      if (done_creating_requests) {
        DM_DBG(DM_N(3), FTHREAD"unlocking mutex before exiting\n", tp_tid, tid);
	S_V(&request_mtx);
        DM_DBG(DM_N(3), FTHREAD"exiting\n", tp_tid, tid);
	pthread_exit(NULL);
      } else {
        DM_DBG(DM_N(3), FTHREAD"going to sleep\n", tp_tid, tid);
      }

      /* Wait for a request to arrive. Note the mutex will be */
      /* unlocked here, thus allowing other threads access to */
      /* requests list.                                       */
      DM_DBG(DM_N(3), FTHREAD"before pthread_cond_wait\n", tp_tid, tid);
      rc = pthread_cond_wait(&request_cv, &request_mtx);
      /* and after we return from pthread_cond_wait, the mutex  */
      /* is locked again, so we don't need to lock it ourselves */
      DM_DBG(DM_N(3), FTHREAD"after pthread_cond_wait\n", tp_tid, tid);
    }

  }
}

static int
tp_create()
{
  DM_DBG_I;
  int i; /* loop counter */

  DM_DBG(DM_N(3), "creating thread pool\n");
  /* create the request-handling threads */
  for (i = 0; i < tp_size; i++) {
    int thread_rval;
    thr_id[i] = i;
    thread_rval = pthread_create(&(p_threads[i]), NULL, request_loop_handler, (void *) &(thr_id[i]));
    if(thread_rval)
    {
      DM_ERR(ERR_SYSTEM, _("failed to create new thread[%i]: %s\n"), i, strerror(errno));
    } else {
      DM_DBG(DM_N(4), "created "FTHREAD"\n", i, p_threads[i]);
      threads_total++;
    }
  }
  DM_DBG(DM_N(3), "created thread pool, threads_total=%d\n", threads_total);

  RETURN(ERR_OK);
}

extern int
tp_init(int threads)
{
  DM_DBG_I;

  tp_size = threads; 
  pthread_mutex_init(&threads_total_mtx, NULL);
  tp_create();

  RETURN(ERR_OK);
}

extern int
tp_cleanup(void)
{
  DM_DBG_I;

  /* The main thread modifies the flag to tell its handler */
  /* Threads no new requests will be generated.            */
  /* Notify our threads we're done creating requests.      */
  int i; /* loop counter */
  int rc;

  rc = S_P(&request_mtx);
  done_creating_requests = 1;
  rc = pthread_cond_broadcast(&request_cv);
  rc = S_V(&request_mtx);

  DM_DBG(DM_N(3), "threads=%i, requests=%d\n", threads_total, num_requests);

  /* Use pthread_join() to wait for all threads to terminate. */
  for (i = 0; i < tp_size; i++) {
    void *thr_retval;

    DM_DBG(DM_N(4), "joining thread (%d/%lu)\n", i, p_threads[i]);
    pthread_join(p_threads[i], &thr_retval);
    threads_total--;
  }
  DM_DBG(DM_N(3), "threads=%i, requests=%d\n", threads_total, num_requests);

  RETURN(ERR_OK);
}

extern int
tp_enqueue(void *data,
           int *sreqs,
           pthread_mutex_t *p_sreqs_mtx,
           pthread_cond_t *p_sreqs_cv)
{
  DM_DBG_I;

  if(sreqs) DM_DBG(DM_N(4), "data=%p; subrequests=%d\n", data, *sreqs);
  else DM_DBG(DM_N(4), "data=%p\n", data);

  int rc = request_add(data, sreqs, p_sreqs_mtx, p_sreqs_cv);

  RETURN(rc);
}

extern int
tp_dequeue(void *data)
{
  DM_DBG_I;

  int rc = request_del(data);

  RETURN(rc);
}

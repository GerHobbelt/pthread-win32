/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
pthread_create (pthread_t * tid,
		const pthread_attr_t * attr,
		void *(*start) (void *),
		void *arg)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function creates a thread running the start function,
      *      passing it the parameter value, 'arg'.
      *
      * PARAMETERS
      *      tid
      *              pointer to an instance of pthread_t
      *
      *      attr
      *              optional pointer to an instance of pthread_attr_t
      *
      *      start
      *              pointer to the starting routine for the new thread
      *
      *      arg
      *              optional parameter passed to 'start'
      *
      *
      * DESCRIPTION
      *      This function creates a thread running the start function,
      *      passing it the parameter value, 'arg'. The 'attr'
      *      argument specifies optional creation attributes.
      *      The thread is identity of the new thread is returned
      *      as 'tid'
      *
      * RESULTS
      *              0               successfully created thread,
      *              EINVAL          attr invalid,
      *              EAGAIN          insufficient resources.
      *
      * ------------------------------------------------------
      */
{
  pthread_t thread;
  int result = EAGAIN;
  int run = TRUE;
  ThreadParms *parms;
  long stackSize;

  if ((thread = (pthread_t) calloc (1, sizeof (*thread))) ==
      NULL)
    {
      goto FAIL0;
    }
  thread->cancelEvent =
    CreateEvent (
		  0,
		  (int) TRUE,	/* manualReset  */
		  (int) FALSE,	/* setSignaled  */
		  NULL);

  if (thread->cancelEvent == NULL)
    {
      goto FAIL0;
    }

  if ((parms = (ThreadParms *) malloc (sizeof (*parms))) ==
      NULL)
    {
      goto FAIL0;
    }

  parms->tid = thread;
  parms->start = start;
  parms->arg = arg;

  if (attr != NULL && *attr != NULL)
    {
      stackSize = (*attr)->stacksize;
      thread->detachState = (*attr)->detachstate;
    }
  else
    {
      /*
       * Default stackSize
       */
      stackSize = 0;
    }

  thread->state = run
    ? PThreadStateInitial
    : PThreadStateSuspended;

  thread->keys = NULL;
  thread->threadH = (HANDLE)
    _beginthreadex (
		     (void *) NULL,	/* No security info             */
		     (unsigned) stackSize,	/* default stack size   */
		     (unsigned (__stdcall *) (void *)) _pthread_threadStart,
		     parms,
		     (unsigned) run ? 0 : CREATE_SUSPENDED,
		     (unsigned *) &(thread->thread));

  result = (thread->threadH != 0) ? 0 : EAGAIN;

  /*
   * Fall Through Intentionally
   */

  /*
   * ------------
   * Failure Code
   * ------------
   */

FAIL0:
  if (result != 0)
    {

      _pthread_threadDestroy (thread);
      thread = NULL;

      if (parms != NULL)
	{
	  free (parms);
	}
    }
  *tid = thread;

  return (result);

}				/* pthread_create */

/* </JEB> */


#if 0 /* Pre Bossom */

#include <errno.h>

#include <windows.h>
#include <process.h>
#include <string.h>

#include "pthread.h"
#include "implement.h"

unsigned
STDCALL _pthread_start_call(void * us_arg)
{
  /* We're now in a running thread. Any local variables here are on
     this thread's private stack so we're safe to leave data in them
     until we leave. */
  pthread_t us;

  /* FIXME: Needs to be a malloc(PTHREAD_KEYS_MAX) otherwise changing
     _PTHREAD_MAX_KEYS in a later version of the DLL will break older apps.
   */
  void * keys[_PTHREAD_MAX_KEYS];

  unsigned (*func)(void *);
  void * arg;
  unsigned ret;

  us = (pthread_t) us_arg;

  memset(keys, 0, sizeof(keys));

  (void) TlsSetValue(_pthread_threadID_TlsIndex, (LPVOID) us);
  (void) TlsSetValue(_pthread_TSD_keys_TlsIndex, (LPVOID) keys);

  /* FIXME: For now, if priority setting fails then at least ensure
     that our records reflect true reality. */
  if (SetThreadPriority((HANDLE) us->win32handle, us->attr.priority) == FALSE)
    {
      us->attr.priority = GetThreadPriority((HANDLE) us->win32handle);
    }

  func = us->call.routine;
  arg = us->call.arg;

  ret = (*func)(arg);

  _pthread_exit(us, NULL, ret);

  /* Never Reached */
  return 0;
}

int
pthread_create(pthread_t *thread, 
	       const pthread_attr_t *attr,
	       void * (*start_routine) (void *), 
	       void * arg)
{
  HANDLE   handle = (HANDLE) NULL;
  unsigned flags;
  void *   security = NULL;
  DWORD  threadID;
  pthread_attr_t * attr_copy;
  pthread_t new_thread;
  /* Success unless otherwise set. */
  int ret;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  ret = _pthread_new_thread(&new_thread);

  pthread_mutex_unlock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  if (ret == 0)
    {
      attr_copy = &(new_thread->attr);

      /* Map given attributes otherwise just use default values. */
      if (attr != NULL) 
	{
	  if (attr_copy->stacksize == 0)
	    {
	      attr_copy->stacksize = PTHREAD_STACK_MIN;
	    }

	  attr_copy->detachstate = attr->detachstate;
	  attr_copy->priority = attr->priority;

#if HAVE_SIGSET_T
	  memcpy(&(attr_copy->sigmask), &(attr->sigmask), sizeof(sigset_t)); 
#endif /* HAVE_SIGSET_T */
	}

      /* We call a generic wrapper which then calls the start routine. */
      new_thread->call.routine = (unsigned (*)(void *)) start_routine;
      new_thread->call.arg = arg;

      /* Start running, not suspended. */
      flags = 0;

      handle = (HANDLE) _beginthreadex(security,
				       attr_copy->stacksize,
				       _pthread_start_call,
				       (void *) new_thread,
				       flags,
				       &threadID);

      if (handle == (HANDLE) NULL)
	{
	  ret = EAGAIN;
	}
    }
  else
    {
      ret = EAGAIN;
    }

  if (ret == 0)
    {
      /* Let the caller know the thread handle. */
      new_thread->win32handle = handle;
      new_thread->ptstatus = _PTHREAD_INUSE;
      *thread = new_thread;
    }
  else
    {
      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      /* Remove the failed thread entry. */
      _pthread_delete_thread(new_thread);

      pthread_mutex_unlock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */
    }

  return ret;
}

#endif /* Pre Bossom */

/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include "pthread.h"
#include "implement.h"

#include <sys/timeb.h>

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
_pthread_processInitialize (void)
     /*
      * ------------------------------------------------------
      * DOCPRIVATE
      *      This function performs process wide initialization for
      *      the pthread library.
      *
      * PARAMETERS
      *      N/A
      *
      * DESCRIPTION
      *      This function performs process wide initialization for
      *      the pthread library.
      *      If successful, this routine sets the global variable
      *      _pthread_processInitialized to TRUE.
      *
      * RESULTS
      *              TRUE    if successful,
      *              FALSE   otherwise
      *
      * ------------------------------------------------------
      */
{
  _pthread_processInitialized = TRUE;

  /*
   * Initialize Keys
   */
  if ((pthread_key_create (&_pthread_selfThreadKey, NULL) != 0) ||
      (pthread_key_create (&_pthread_cleanupKey, NULL) != 0))
    {

      _pthread_processTerminate ();
    }

  /* 
   * Set up the global mutex test and init check lock.
   */
  InitializeCriticalSection(&_pthread_mutex_test_init_lock);

  return (_pthread_processInitialized);

}				/* processInitialize */

void
_pthread_processTerminate (void)
     /*
      * ------------------------------------------------------
      * DOCPRIVATE
      *      This function performs process wide termination for
      *      the pthread library.
      *
      * PARAMETERS
      *      N/A
      *
      * DESCRIPTION
      *      This function performs process wide termination for
      *      the pthread library.
      *      This routine sets the global variable
      *      _pthread_processInitialized to FALSE
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  if (_pthread_processInitialized)
    {

      if (_pthread_selfThreadKey != NULL)
	{
	  /*
	   * Release _pthread_selfThreadKey
	   */
	  pthread_key_delete (_pthread_selfThreadKey);

	  _pthread_selfThreadKey = NULL;
	}

      if (_pthread_cleanupKey != NULL)
	{
	  /*
	   * Release _pthread_cleanupKey
	   */
	  pthread_key_delete (_pthread_cleanupKey);

	  _pthread_cleanupKey = NULL;
	}

      /* 
       * Destroy up the global mutex test and init check lock.
       */
      DeleteCriticalSection(&_pthread_mutex_test_init_lock);

      _pthread_processInitialized = FALSE;
    }

}				/* processTerminate */

void *
_pthread_threadStart (ThreadParms * threadParms)
{
  pthread_t tid;
  void *(*start) (void *);
  void *arg;

  int status;

  tid = threadParms->tid;
  start = threadParms->start;
  arg = threadParms->arg;

  free (threadParms);

  pthread_setspecific (_pthread_selfThreadKey, tid);

#ifdef _WIN32

  __try
  {
    /*
     * Run the caller's routine;
     */
    (*start) (arg);
    status = 0;
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    /*
     * A system unexpected exception had occurred running the user's
     * routine. We get control back within this block.
     */
    status = -1;
  }

#else /* _WIN32 */

#ifdef __cplusplus

  try
  {
    /*
     * Run the caller's routine;
     */
    (*start) (arg);
    status = 0;
  }
  catch (...)
  {
    /*
     * A system unexpected exception had occurred running the user's
     * routine. We get control back within this block.
     */
    status = -1;
  }

#else /* __cplusplus */

#if defined(__CYGWIN__) || defined(__CYGWIN32__)
#warning Compile __FILE__ as C++ or thread cancellation will not work properly.
#endif

  /*
   * Run the caller's routine;
   */
  (*start) (arg);
  status = 0;

#endif /* __cplusplus */

#endif /* _WIN32 */

  pthread_exit ((void *) status);

  return ((void *) status);

}				/* threadStart */

void
_pthread_threadDestroy (pthread_t thread)
{
  if (thread != NULL)
    {
      _pthread_callUserDestroyRoutines (thread);

      if (thread->cancelEvent != NULL)
	{
	  CloseHandle (thread->cancelEvent);
	}

      free (thread);
    }

}				/* threadDestroy */

int
_pthread_tkAssocCreate (ThreadKeyAssoc ** assocP,
			pthread_t thread,
			pthread_key_t key)
     /*
      * -------------------------------------------------------------------
      * This routine creates an association that
      * is unique for the given (thread,key) combination.The association 
      * is referenced by both the thread and the key.
      * This association allows us to determine what keys the
      * current thread references and what threads a given key
      * references.
      * See the detailed description
      * at the beginning of this file for further details.
      *
      * Notes:
      *      1)      New associations are pushed to the beginning of the
      *              chain so that the internal _pthread_selfThreadKey association
      *              is always last, thus allowing selfThreadExit to
      *              be implicitly called by pthread_exit last.
      *
      * Parameters:
      *              assocP
      *                      address into which the association is returned.
      *              thread
      *                      current running thread. If NULL, then association
      *                      is only added to the key. A NULL thread indicates
      *                      that the user called pthread_setspecific prior
      *                      to starting a thread. That's ok.
      *              key
      *                      key on which to create an association.
      * Returns:
      *       0              - if successful,
      *      -1              - general error
      * -------------------------------------------------------------------
      */
{
  int result;
  ThreadKeyAssoc *assoc;

  /*
   * Have to create an association and add it
   * to both the key and the thread.
   */
  assoc = (ThreadKeyAssoc *)
    calloc (1, sizeof (*assoc));

  if (assoc == NULL)
    {
      result = -1;
      goto FAIL0;
    }

  if ((result = pthread_mutex_init (&(assoc->lock), NULL)) !=
      0)
    {
      goto FAIL1;
    }

  assoc->thread = thread;
  assoc->key = key;

  /*
   * Register assoc with key
   */
  if ((result = pthread_mutex_lock (&(key->threadsLock))) !=
      0)
    {
      goto FAIL2;
    }

  assoc->nextThread = (ThreadKeyAssoc *) key->threads;
  key->threads = (void *) assoc;

  pthread_mutex_unlock (&(key->threadsLock));

  if (thread != NULL)
    {
      /*
       * Register assoc with thread
       */
      assoc->nextKey = (ThreadKeyAssoc *) thread->keys;
      thread->keys = (void *) assoc;
    }

  *assocP = assoc;

  return (result);

  /*
   * -------------
   * Failure Code
   * -------------
   */
FAIL2:
  pthread_mutex_destroy (&(assoc->lock));

FAIL1:
  free (assoc);

FAIL0:

  return (result);

}				/* tkAssocCreate */


void
_pthread_tkAssocDestroy (ThreadKeyAssoc * assoc)
     /*
      * -------------------------------------------------------------------
      * This routine releases all resources for the given ThreadKeyAssoc
      * once it is no longer being referenced
      * ie) both the key and thread have stopped referencing it.
      *
      * Parameters:
      *              assoc
      *                      an instance of ThreadKeyAssoc.
      * Returns:
      *      N/A
      * -------------------------------------------------------------------
      */
{

  if ((assoc != NULL) &&
      (assoc->key == NULL && assoc->thread == NULL))
    {

      pthread_mutex_destroy (&(assoc->lock));

      free (assoc);
    }

}				/* tkAssocDestroy */


void
_pthread_callUserDestroyRoutines (pthread_t thread)
     /*
      * -------------------------------------------------------------------
      * DOCPRIVATE
      *
      * This the routine runs through all thread keys and calls
      * the destroy routines on the user's data for the current thread.
      * It simulates the behaviour of POSIX Threads.
      *
      * PARAMETERS
      *              thread
      *                      an instance of pthread_t
      *
      * RETURNS
      *              N/A
      * -------------------------------------------------------------------
      */
{
  ThreadKeyAssoc **nextP;
  ThreadKeyAssoc *assoc;

  if (thread != NULL)
    {
      /*
       * Run through all Thread<-->Key associations
       * for the current thread.
       * If the pthread_key_t still exits (ie the assoc->key
       * is not NULL) then call the user's TSD destroy routine.
       * Notes:
       *      If assoc->key is NULL, then the user previously called
       *      PThreadKeyDestroy. The association is now only referenced
       *      by the current thread and must be released; otherwise
       *      the assoc will be destroyed when the key is destroyed.
       */
      nextP = (ThreadKeyAssoc **) & (thread->keys);
      assoc = *nextP;

      while (assoc != NULL)
	{

	  if (pthread_mutex_lock (&(assoc->lock)) == 0)
	    {
	      pthread_key_t k;
	      if ((k = assoc->key) != NULL)
		{
		  /*
		   * Key still active; pthread_key_delete
		   * will block on this same mutex before
		   * it can release actual key; therefore,
		   * key is valid and we can call the destroy
		   * routine;
		   */
		  void *value = NULL;

		  value = pthread_getspecific (k);
		  if (value != NULL && k->destructor != NULL)
		    {

		      __try
		      {
			/*
			 * Run the caller's cleanup routine.
			 */
			(*(k->destructor)) (value);
		      }
		      __except (EXCEPTION_EXECUTE_HANDLER)
		      {
			/*
			 * A system unexpected exception had occurred
			 * running the user's destructor.
			 * We get control back within this block.
			 */
		      }
		    }
		}

	      /*
	       * mark assoc->thread as NULL to indicate the
	       * thread no longer references this association
	       */
	      assoc->thread = NULL;

	      /*
	       * Remove association from the pthread_t chain
	       */
	      *nextP = assoc->nextKey;

	      pthread_mutex_unlock (&(assoc->lock));

	      _pthread_tkAssocDestroy (assoc);

	      assoc = *nextP;
	    }
	}
    }

}				/* callUserDestroyRoutines */

/* </JEB> */


int
_pthread_sem_timedwait (sem_t * sem, const struct timespec * abstime)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function waits on a semaphore possibly until
      *      'abstime' time.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *      abstime
      *              pointer to an instance of struct timespec
      *
      * DESCRIPTION
      *      This function waits on a semaphore. If the
      *      semaphore value is greater than zero, it decreases
      *      its value by one. If the semaphore value is zero, then
      *      the calling thread (or process) is blocked until it can
      *      successfully decrease the value or until interrupted by
      *      a signal.
      *
      *      If 'abstime' is a NULL pointer then this function will
      *      block until it can successfully decrease the value or
      *      until interrupted by a signal.
      *
      * RESULTS
      *              0               successfully decreased semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *              ETIMEDOUT       abstime elapsed before success.
      *
      * ------------------------------------------------------
      */
{
  struct _timeb currSysTime;
  const DWORD NANOSEC_PER_MILLISEC = 1000000;
  const DWORD MILLISEC_PER_SEC = 1000;
  DWORD milliseconds;

  if (abstime == NULL)
    {
      milliseconds = INFINITE;
    }
  else
    {
      /* 
       * Calculate timeout as milliseconds from current system time. 
       */

      /* get current system time */
      _ftime(&currSysTime);

      /* subtract current system time from abstime */
      milliseconds = (abstime->tv_sec - currSysTime.time) * MILLISEC_PER_SEC;
      milliseconds += (abstime->tv_nsec / NANOSEC_PER_MILLISEC) -
	currSysTime.millitm;
    }

  return ((sem == NULL)
	  ? EINVAL
	  : pthreadCancelableTimedWait (*sem, milliseconds)
    );

}				/* _pthread_sem_timedwait */



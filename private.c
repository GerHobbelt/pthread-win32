/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

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

#if defined( KLUDGE )

void
_pthread_cleanupStack (void)
{
  while (pthread_pop_cleanup (1))
    {
    }

}				/* cleanupStack */

#endif /* KLUDGE */

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


#if 0 /* Pre Bossom */

/* Thread ID management.
   ---------------------

   We started by simply mapping the Win32 thread handle directly to
   pthread_t. However, in order to process pthread_join()'s, we need
   to be able to keep our POSIX thread ID (pthread_t) around after the
   Win32 thread has terminated. Win32 may reuse the Win32 handle during that
   time, which will conflict.

   The pthread_t value is now actually the pointer to a thread struct:

   typedef struct _pthread * pthread_t;

   which amongst other things stores the Win32 thread handle:

   struct _pthread {
     HANDLE  win32handle;
     int     ptstatus;
     ...
   };

   So now whereever we need to use the Win32 handle it can be accessed
   as:

   pthread_t T = pthread_this();
   HANDLE    H;

   H = T->win32handle;

   // or (which is NOT preferred, let the compiler optimise to this).

   H = (HANDLE) *T;


   POSIX Threads Table
   -------------------

   Having the thread ID as a pointer to the thread struct itself
   avoids the need to search the threads table in all but the initial
   occasion where we create the thread.

   Initially we used a hash function to select a free thread struct
   from the table, possibly needing a walk through the table if the
   hash collided with an already in-use thread.

   The scheme used now is more efficient and is done as follows:

   We use two tables and two counters:

   struct _pthread  _pthread_virgins[PTHREAD_THREADS_MAX];
   pthread_t        _pthread_reuse[PTHREAD_THREADS_MAX];

   int       _pthread_virgin_next = 0;
   int       _pthread_reuse_top = -1;

   The counter _pthread_virgin_next is an index into _pthread_virgins[],
   which can be thought of as a list, and _pthread_reuse_top is an
   index into _pthread_reuse[], which can be thought of as a LIFO stack.

   Once taken from _pthread_virgins[], used and freed threads are only
   ever pushed back onto _pthread_reuse[].

 */

int
_pthread_new_thread(pthread_t * thread)
{
  pthread_t new_thread;

  if (_pthread_reuse_top >= 0)
    {
      new_thread = _pthread_reuse[_pthread_reuse_top--];
    }
  else
    {
      if (_pthread_virgin_next < PTHREAD_THREADS_MAX)
	{
	  new_thread = (pthread_t) &_pthread_virgins[_pthread_virgin_next++];
	}
      else
	{
	  return EAGAIN;
	}
    }

  new_thread->win32handle = (HANDLE) NULL;
  new_thread->ptstatus = _PTHREAD_NEW;
  pthread_attr_init(&(new_thread->attr));
  new_thread->joinvalueptr = NULL;
  new_thread->cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread->cancel_pending = FALSE;
  new_thread->cleanupstack = NULL;
  new_thread->forkpreparestack = NULL;
  new_thread->forkparentstack = NULL;
  new_thread->forkchildstack = NULL;

  *thread = new_thread;
  _pthread_threads_count++;

  return 0;
}

int
_pthread_delete_thread(_pthread_t * thread)
{
  /* We don't check that the thread has been properly cleaned up, so
     it had better be done already. */

  /* Release any keys */

  _pthread_destructor_run_all();

  /* Remove the thread entry if necessary. */

  if (thread != NULL
      && thread->ptstatus == _PTHREAD_EXITED)
    {
      pthread_attr_destroy(&(thread->attr));
      thread->win32handle = (HANDLE) NULL;
      thread->ptstatus = _PTHREAD_REUSE;

      _pthread_reuse[++_pthread_reuse_top] = thread;
      _pthread_threads_count--;

      return 0;
    }

  return EINVAL;
}

#endif /* Pre Bossom */

/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#if !defined(_MSC_VER) && !defined(__cplusplus) && defined(__GNUC__)

#warning Compile __FILE__ as C++ or thread cancellation will not work properly.

#endif /* !_MSC_VER && !__cplusplus && __GNUC__ */

#ifndef NEED_FTIME
#include <sys/timeb.h>
#endif
#include "pthread.h"
#include "semaphore.h"
#include "implement.h"


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
	if (_pthread_processInitialized) {
		/* 
		 * ignore if already initialized. this is useful for 
		 * programs that uses a non-dll pthread
		 * library. such programs must call _pthread_processInitialize() explicitely,
		 * since this initialization routine is automatically called only when
		 * the dll is loaded.
		 */
		return TRUE;
	}

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
   * Set up the global test and init check locks.
   */
  InitializeCriticalSection(&_pthread_mutex_test_init_lock);
  InitializeCriticalSection(&_pthread_cond_test_init_lock);
  InitializeCriticalSection(&_pthread_rwlock_test_init_lock);

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
       * Destroy the global test and init check locks.
       */
      DeleteCriticalSection(&_pthread_rwlock_test_init_lock);
      DeleteCriticalSection(&_pthread_cond_test_init_lock);
      DeleteCriticalSection(&_pthread_mutex_test_init_lock);

      _pthread_processInitialized = FALSE;
    }

}				/* processTerminate */

#ifdef _MSC_VER

static DWORD
ExceptionFilter (EXCEPTION_POINTERS * ep, DWORD * ei)
{
  DWORD param;
  DWORD numParams = ep->ExceptionRecord->NumberParameters;
  
  numParams = (numParams > 3) ? 3 : numParams;

  for (param = 0; param < numParams; param++)
    {
      ei[param] = ep->ExceptionRecord->ExceptionInformation[param];
    }

  return EXCEPTION_EXECUTE_HANDLER;
}

#endif /* _MSC_VER */

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
unsigned PT_STDCALL
#else
void
#endif
_pthread_threadStart (ThreadParms * threadParms)
{
  pthread_t self;
  void *(*start) (void *);
  void *arg;

#ifdef _MSC_VER

  DWORD ei[3];

#endif

  void * status;

  self = threadParms->tid;
  start = threadParms->start;
  arg = threadParms->arg;

#if defined (__MINGW32__) && ! defined (__MSVCRT__)
  /* beginthread does not return the thread id, and so we do it here. */
  self->thread = GetCurrentThreadId ();
#endif

  free (threadParms);

  pthread_setspecific (_pthread_selfThreadKey, self);

#ifdef _MSC_VER

  __try
  {
    /*
     * Run the caller's routine;
     */
    status = (*start) (arg);
  }
  _pthread__except (ExceptionFilter(GetExceptionInformation(), ei))
  {
    DWORD ec = GetExceptionCode();

    if (ec == EXCEPTION_PTHREAD_SERVICES)
      {
	switch (ei[0])
	  {
	  case _PTHREAD_EPS_CANCEL:
	    status = PTHREAD_CANCELED;
	    break;
	  case _PTHREAD_EPS_EXIT:
	    status = (void *) ei[1];
	    break;
	  default:
	    status = PTHREAD_CANCELED;
	  }
      }
    else
      {
	/*
	 * A system unexpected exception had occurred running the user's
	 * routine. We get control back within this block.
	 */
	status = PTHREAD_CANCELED;
      }
  }

#else /* _MSC_VER */

#ifdef __cplusplus

  try
  {
    /*
     * Run the caller's routine;
     */
    status = self->exitStatus = (*start) (arg);
  }
  _pthread_catch (Pthread_exception_cancel)
    {
      /*
       * Thread was cancelled.
       */
      status = PTHREAD_CANCELED;
    }
  _pthread_catch (Pthread_exception_exit)
    {
      /*
       * Thread was exited via pthread_exit().
       */
      status = self->exitStatus;
    }
  _pthread_catch (...)
    {
      /*
       * A system unexpected exception had occurred running the user's
       * routine. We get control back within this block.
       */
      status = PTHREAD_CANCELED;
    }

#else /* __cplusplus */

  /*
   * Run the caller's routine; no cancelation or other exceptions will
   * be honoured.
   */
  status = (*start) (arg);

#endif /* __cplusplus */

#endif /* _MSC_VER */

  (void) pthread_mutex_destroy(&self->cancelLock);
  _pthread_callUserDestroyRoutines(self);

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
  _endthreadex ((unsigned) status);
#else
  _endthread ();
#endif

  /*
   * Never reached.
   */

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
  return (unsigned) status;
#endif

}				/* _pthread_threadStart */

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

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
      /* See documentation for endthread vs endthreadex. */
      if( thread->threadH != 0 )
	{
	  CloseHandle( thread->threadH );
	}
#endif

      free (thread);
    }

}				/* _pthread_threadDestroy */

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

}				/* _pthread_tkAssocCreate */


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

}				/* _pthread_tkAssocDestroy */


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

#ifdef _MSC_VER

		      __try
		      {
			/*
			 * Run the caller's cleanup routine.
			 */
			(*(k->destructor)) (value);
		      }
		      _pthread__except (EXCEPTION_EXECUTE_HANDLER)
		      {
			/*
			 * A system unexpected exception had occurred
			 * running the user's destructor.
			 * We get control back within this block.
			 */
		      }

#else  /* _MSC_VER */
#ifdef __cplusplus

		      try
		      {
			/*
			 * Run the caller's cleanup routine.
			 */
			(*(k->destructor)) (value);
		      }
		      _pthread_catch (...)
		      {
			/*
			 * A system unexpected exception had occurred
			 * running the user's destructor.
			 * We get control back within this block.
			 */
		      }

#else  /* __cplusplus */

			/*
			 * Run the caller's cleanup routine.
			 */
			(*(k->destructor)) (value);

#endif /* __cplusplus */
#endif /* _MSC_VER */
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

}				/* _pthread_callUserDestroyRoutines */



#ifdef NEED_FTIME

/*
 * time between jan 1, 1601 and jan 1, 1970 in units of 100 nanoseconds
 */
#define TIMESPEC_TO_FILETIME_OFFSET \
          ( ((LONGLONG) 27111902 << 32) + (LONGLONG) 3577643008 )

static void
timespec_to_filetime(const struct timespec *ts, FILETIME *ft)
     /*
      * -------------------------------------------------------------------
      * converts struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * into FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * -------------------------------------------------------------------
      */
{
	*(LONGLONG *)ft = ts->tv_sec * 10000000 + (ts->tv_nsec + 50) / 100 + TIMESPEC_TO_FILETIME_OFFSET;
}

static void
filetime_to_timespec(const FILETIME *ft, struct timespec *ts)
     /*
      * -------------------------------------------------------------------
      * converts FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * into struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * -------------------------------------------------------------------
      */
{
	ts->tv_sec = (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
	ts->tv_nsec = (int)((*(LONGLONG *)ft - TIMESPEC_TO_FILETIME_OFFSET - ((LONGLONG)ts->tv_sec * (LONGLONG)10000000)) * 100);
}

#endif /* NEED_FTIME */

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
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *              ETIMEDOUT       abstime elapsed before success.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

#ifdef NEED_FTIME

  struct timespec currSysTime;

#else /* NEED_FTIME */

  struct _timeb currSysTime;

#endif /* NEED_FTIME */

  const DWORD NANOSEC_PER_MILLISEC = 1000000;
  const DWORD MILLISEC_PER_SEC = 1000;
  DWORD milliseconds;

  if (sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
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

#ifdef NEED_FTIME

	  {
	    FILETIME ft;
	    SYSTEMTIME st;

	    GetSystemTime(&st);
            SystemTimeToFileTime(&st, &ft);
	    /*
             * GetSystemTimeAsFileTime(&ft); would be faster,
             * but it does not exist on WinCE
             */

	    filetime_to_timespec(&ft, &currSysTime);
	  }

	  /*
           * subtract current system time from abstime
           */
	  milliseconds = (abstime->tv_sec - currSysTime.tv_sec) * MILLISEC_PER_SEC;
	  milliseconds += ((abstime->tv_nsec - currSysTime.tv_nsec) + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC;

#else /* NEED_FTIME */
	  _ftime(&currSysTime);

	  /*
           * subtract current system time from abstime
           */
	  milliseconds = (abstime->tv_sec - currSysTime.time) * MILLISEC_PER_SEC;
	  milliseconds += ((abstime->tv_nsec + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC) -
	    currSysTime.millitm;

#endif /* NEED_FTIME */


	  if (((int) milliseconds) < 0)
	    milliseconds = 0;
	}

#ifdef NEED_SEM

      result = (pthreadCancelableTimedWait (sem->event, milliseconds));

#else /* NEED_SEM */

      result = (pthreadCancelableTimedWait (*sem, milliseconds));

#endif

    }

  if (result != 0)
    {

      errno = result;
      return -1;

    }

#ifdef NEED_SEM

  _pthread_decrease_semaphore(sem);

#endif /* NEED_SEM */

  return 0;

}				/* _pthread_sem_timedwait */


DWORD
_pthread_get_exception_services_code(void)
{
#ifdef _MSC_VER

  return EXCEPTION_PTHREAD_SERVICES;

#else

  return NULL;

#endif
}

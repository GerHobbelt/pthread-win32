/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
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

#include "pthread.h"
#include "implement.h"
#ifndef _UWIN
#include <process.h>
#endif

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
  HANDLE threadH = 0;
  int result = EAGAIN;
  int run = TRUE;
  ThreadParms *parms = NULL;
  long stackSize;

  if ((thread = ptw32_new()) == NULL)
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

  if ((parms = (ThreadParms *) malloc (sizeof (*parms))) == NULL)
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

#if HAVE_SIGSET_T

      thread->sigmask = (*attr)->sigmask;

#endif /* HAVE_SIGSET_T */

    }
  else
    {
      /*
       * Default stackSize
       */
      stackSize = PTHREAD_STACK_MIN;
    }

  thread->state = run
    ? PThreadStateInitial
    : PThreadStateSuspended;

  thread->keys = NULL;

  /*
   * Threads must be started in suspended mode and resumed if necessary
   * after _beginthreadex returns us the handle. Otherwise we set up a
   * race condition between the creating and the created threads.
   * Note that we also retain a local copy of the handle for use
   * by us in case thread->threadH gets NULLed later but before we've
   * finished with it here.
   */

#if ! defined (__MINGW32__) || defined (__MSVCRT__)

  thread->threadH = threadH = (HANDLE)
    _beginthreadex (
                    (void *) NULL,	/* No security info             */
                    (unsigned) stackSize,	/* default stack size   */
                    ptw32_threadStart,
                    parms,
                    (unsigned) CREATE_SUSPENDED,
                    (unsigned *) &(thread->thread));

  if (threadH != 0)
    {
      /*
       * PTHREAD_EXPLICIT_SCHED is the default because Win32 threads
       * don't inherit their creator's priority. They are started with
       * THREAD_PRIORITY_NORMAL (win32 value). The result of not supplying
       * an 'attr' arg to pthread_create() is equivalent to defaulting to
       * PTHREAD_EXPLICIT_SCHED and priority THREAD_PRIORITY_NORMAL.
       */
      if (attr != NULL && *attr != NULL)
        {
          (void) SetThreadPriority(thread->threadH,
                                   PTHREAD_INHERIT_SCHED == (*attr)->inheritsched
                                   ? GetThreadPriority(GetCurrentThread())
                                   : (*attr)->param.sched_priority );
        }

      if (run)
        {
          ResumeThread(threadH);
        }
    }

#else /* __MINGW32__ && ! __MSVCRT__ */

  /*
   * This lock will force pthread_threadStart() to wait until we have
   * the thread handle.
   */
  (void) pthread_mutex_lock(&thread->cancelLock);

  thread->threadH = threadH = (HANDLE)
    _beginthread (
                  ptw32_threadStart,
                  (unsigned) stackSize,	/* default stack size   */
                  parms);

  /*
   * Make the return code match _beginthreadex's.
   */
  if (threadH == (HANDLE) -1L)
    {
      thread->threadH = threadH = 0;
    }
  else
    {
      if (! run)
        {
          /* 
           * beginthread does not allow for create flags, so we do it now.
           * Note that beginthread itself creates the thread in SUSPENDED
           * mode, and then calls ResumeThread to start it.
           */
          SuspendThread (threadH);
        }
      
      /*
       * PTHREAD_EXPLICIT_SCHED is the default because Win32 threads
       * don't inherit their creator's priority. They are started with
       * THREAD_PRIORITY_NORMAL (win32 value). The result of not supplying
       * an 'attr' arg to pthread_create() is equivalent to defaulting to
       * PTHREAD_EXPLICIT_SCHED and priority THREAD_PRIORITY_NORMAL.
       */
      if (attr != NULL && *attr != NULL)
        {
          (void) SetThreadPriority(thread->threadH,
                                   PTHREAD_INHERIT_SCHED == (*attr)->inheritsched
                                   ? GetThreadPriority(GetCurrentThread())
                                   : (*attr)->param.sched_priority );
        }
    }

  (void) pthread_mutex_unlock(&thread->cancelLock);

#endif /* __MINGW32__ && ! __MSVCRT__ */

  result = (threadH != 0) ? 0 : EAGAIN;

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

      ptw32_threadDestroy (thread);
      thread = NULL;

      if (parms != NULL)
	{
	  free (parms);
	}
    }
  *tid = thread;

#ifdef _UWIN
	if (result == 0)
		pthread_count++;
#endif
  return (result);

}				/* pthread_create */


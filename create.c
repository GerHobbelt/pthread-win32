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
  ThreadParms *parms = NULL;
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

#if HAVE_SIGSET_T

      thread->sigmask = (*attr)->sigmask;

#endif /* HAVE_SIGSET_T */

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
		     (unsigned (PT_STDCALL *) (void *)) _pthread_threadStart,
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


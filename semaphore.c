/*
 * -------------------------------------------------------------
 *
 * Module: semaphore.c
 *
 * Purpose:
 *      Semaphores aren't actually part of the PThreads standard.
 *      They are defined by the POSIX Standard:
 *
 *              POSIX 1003.1b-1993      (POSIX.1b)
 *
 * -------------------------------------------------------------
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

/* ignore warning "unreferenced formal parameter" */
#ifdef _MSC_VER
#pragma warning( disable : 4100 )
#endif

#include "pthread.h"
#include "semaphore.h"
#include "implement.h"

int
sem_init (sem_t * sem, int pshared, unsigned int value)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function initializes an unnamed semaphore. the
      *      initial value of the semaphore is 'value'
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *      pshared
      *              if zero, this semaphore may only be shared between
      *              threads in the same process.
      *              if nonzero, the semaphore can be shared between
      *              processes
      *
      *      value
      *              initial value of the semaphore counter
      *
      * DESCRIPTION
      *      This function initializes an unnamed semaphore. The
      *      initial value of the semaphore is set to 'value'.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSPC          a required resource has been exhausted,
      *              ENOSYS          semaphores are not supported,
      *              EPERM           the process lacks appropriate privilege
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  sem_t s;

  if (pshared != 0)
    {
      /*
       * Creating a semaphore that can be shared between
       * processes
       */
      result = EPERM;

    }
  else
    {
      s = (sem_t) calloc (1, sizeof (*s));

#ifdef NEED_SEM

      if (NULL == s)
        {
          result = ENOMEM;
        }
      else
        {
          s->value = value;
          s->event = CreateEvent (NULL,
                                  FALSE,	/* manual reset */
                                  FALSE,	/* initial state */
                                  NULL);
          if (0 == s->Event)
            {
              result = ENOSPC;
            }
          else
            {
              if (value != 0)
                {
                  SetEvent(s->event);
                }

              InitializeCriticalSection(&s->sem_lock_cs);
            }

#else /* NEED_SEM */

      s->sem = CreateSemaphore (NULL,        /* Always NULL */
                                value,       /* Initial value */
                                0x7FFFFFFFL, /* Maximum value */
                                NULL);       /* Name */

      if (0 == s->sem)
	{
	  result = ENOSPC;
	}

#endif /* NEED_SEM */

    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  *sem = s;

  return 0;

}				/* sem_init */


int
sem_destroy (sem_t * sem)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function destroys an unnamed semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      * DESCRIPTION
      *      This function destroys an unnamed semaphore.
      *
      * RESULTS
      *              0               successfully destroyed semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EBUSY           threads (or processes) are currently
      *                                      blocked on 'sem'
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  sem_t s;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
	s = *sem;
      *sem = NULL;

#ifdef NEED_SEM

      if (! CloseHandle(s->event))
        {
          *sem = s;
          result = EINVAL;
        }
      else
        {
          DeleteCriticalSection(&s->sem_lock_cs);
          free(s);
        }

#else /* NEED_SEM */

      if (! CloseHandle (s->sem))
        {
          *sem = s;
          result = EINVAL;
        }

#endif /* NEED_SEM */

    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  return 0;

}				/* sem_destroy */


int
sem_trywait (sem_t * sem)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function tries to wait on a semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      * DESCRIPTION
      *      This function tries to wait on a semaphore. If the
      *      semaphore value is greater than zero, it decreases
      *      its value by one. If the semaphore value is zero, then
      *      this function returns immediately with the error EAGAIN
      *
      * RESULTS
      *              0               successfully decreased semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EAGAIN          the semaphore was already locked,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *
      * ------------------------------------------------------
      */
{
#ifdef NEED_SEM

  /*
   * not yet implemented!
   */
  int result = EINVAL;
  return -1;

#else /* NEED_SEM */

  int result = 0;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else if (WaitForSingleObject ((*sem)->sem, 0) == WAIT_TIMEOUT)
    {
      result = EAGAIN;
    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  return 0;

#endif /* NEED_SEM */

}				/* sem_trywait */


#ifdef NEED_SEM

void 
ptw32_decrease_semaphore(sem_t * sem)
{
  register sem_t s = *sem;

  EnterCriticalSection(&s->sem_lock_cs);

  if (s->value != 0)
    {
      s->value--;
      if (s->value != 0)
        {
          SetEvent(s->event);
        }
    }
  else
    {
      /* this case should not happen! */
    }

  LeaveCriticalSection(&s->sem_lock_cs);
}

BOOL 
ptw32_increase_semaphore(sem_t * sem, unsigned int n)
{
  BOOL result;
  register sem_t s = *sem;

  EnterCriticalSection(&s->sem_lock_cs);

  if (s->value + n > s->value)
    {
       s->value += n;
       SetEvent(s->event);
       result = TRUE;
    }
  else
    {
       result = FALSE;
    }

  LeaveCriticalSection(&s->sem_lock_cs);
  return result;
}

#endif /* NEED_SEM */

int
sem_wait (sem_t * sem)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function  waits on a semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      * DESCRIPTION
      *      This function waits on a semaphore. If the
      *      semaphore value is greater than zero, it decreases
      *      its value by one. If the semaphore value is zero, then
      *      the calling thread (or process) is blocked until it can
      *      successfully decrease the value or until interrupted by
      *      a signal.
      *
      * RESULTS
      *              0               successfully decreased semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else
    {

#ifdef NEED_SEM

	result = pthreadCancelableWait ((*sem)->event);

#else /* NEED_SEM */

	result = pthreadCancelableWait ((*sem)->sem);

#endif /* NEED_SEM */

    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

#ifdef NEED_SEM

  ptw32_decrease_semaphore(sem);

#endif /* NEED_SEM */

  return 0;

}				/* sem_wait */


int
sem_post (sem_t * sem)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function posts a wakeup to a semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      * DESCRIPTION
      *      This function posts a wakeup to a semaphore. If there
      *      are waiting threads (or processes), one is awakened;
      *      otherwise, the semaphore value is incremented by one.
      *
      * RESULTS
      *              0               successfully posted semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (sem == NULL || *sem == NULL)
    {
	result = EINVAL;
    }

#ifdef NEED_SEM

  else if (! ptw32_increase_semaphore (sem, 1))

#else /* NEED_SEM */

  else if (! ReleaseSemaphore ((*sem)->sem, 1, 0))

#endif /* NEED_SEM */

    {
	result = EINVAL;
    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  return 0;

}				/* sem_post */


int
sem_post_multiple (sem_t * sem, int count )
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function posts multiple wakeups to a semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *      count
      *              counter, must be greater than zero.
      *
      * DESCRIPTION
      *      This function posts multiple wakeups to a semaphore. If there
      *      are waiting threads (or processes), n <= count are awakened;
      *      the semaphore value is incremented by count - n.
      *
      * RESULTS
      *              0               successfully posted semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore
      *                              or count is less than or equal to zero.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (sem == NULL || *sem == NULL || count <= 0)
    {
      result = EINVAL;
    }

#ifdef NEED_SEM

  else if (! ptw32_increase_semaphore (sem, count))

#else /* NEED_SEM */

  else if (! ReleaseSemaphore ((*sem)->sem, count, 0))

#endif /* NEED_SEM */

    {
      result = EINVAL;
    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  return 0;

}                               /* sem_post_multiple */


int
sem_open (const char * name, int oflag, mode_t mode, unsigned int value)
{
  errno = ENOSYS;
  return -1;
}				/* sem_open */


int
sem_close (sem_t * sem)
{
  errno = ENOSYS;
  return -1;
}				/* sem_close */


int
sem_unlink (const char * name)
{
  errno = ENOSYS;
  return -1;
}				/* sem_unlink */


int
sem_getvalue (sem_t * sem, int * sval)
{
  errno = ENOSYS;
  return -1;
}				/* sem_getvalue */


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
 *      They are supposed to follow the older UNIX convention for
 *      reporting errors. That is, on failure they are supposed
 *      to return a value of -1 and store the appropriate error
 *      number into 'errno'.
 *      HOWEVER,errno cannot be modified in a multithreaded
 *      program on WIN32; therefore, the value is returned as
 *      the function value.
 *      It is recommended that you compare for zero (0) for success
 *      instead of -1 for failure when checking the status of
 *      these functions.
 *
 * Contents:
 *              Public Methods                    Author
 *              --------------                    ------
 *              sem_init                          John E. Bossom  Mar 1998
 *              sem_destroy                       John E. Bossom  Mar 1998
 *              sem_trywait                       John E. Bossom  Mar 1998
 *              sem_wait                          John E. Bossom  Mar 1998
 *              sem_post                          John E. Bossom  Mar 1998
 *
 *              Private Methods
 *              ---------------
 *
 * -------------------------------------------------------------
 */
#include <pthread.h>
#include <string.h>

#include "semaphore.h"


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
      *              0                       successfully created semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSPC          a required resource has been exhausted,
      *              ENOSYS          semaphores are not supported,
      *              EPERM           the process lacks appropriate privilege
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

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
      /*
       * NOTE: Taking advantage of the fact that
       *               sem_t is a simple structure with one entry;
       *               We don't have to allocate it...
       */
      *sem = CreateSemaphore (
			       0,
			       value,
			       0x7FFFFFF,
			       NULL);

      if (*sem == 0)
	{
	  result = ENOSPC;
	}
    }

  return (result);

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
      *              0                       successfully destroyed semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EBUSY           threads (or processes) are currently
      *                                      blocked on 'sem'
      *
      * ------------------------------------------------------
      */
{
  return ((sem == NULL)
	  ? EINVAL
	  : (CloseHandle (*sem)
	     ? 0
	     : EINVAL));

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
      *              0                       successfully destroyed semaphore,
      *              EAGAIN          the semaphore was already locked,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *
      * ------------------------------------------------------
      */
{
  return ((sem == NULL)
	  ? EINVAL
	  : ((WaitForSingleObject (*sem, 0) == WAIT_TIMEOUT)
	     ? EAGAIN
	     : 0));

}				/* sem_trywait */


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
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *
      * ------------------------------------------------------
      */
{

  return ((sem == NULL)
	  ? EINVAL
	  : pthreadCancelableWait (*sem)
    );

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
      *              0               successfully destroyed semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *
      * ------------------------------------------------------
      */
{
  return ((sem == NULL)
	  ? EINVAL
	  : (ReleaseSemaphore (*sem, 1, 0)
	     ? 0
	     : EINVAL));

}				/* sem_post */



/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 */

#include "pthread.h"

int
pthread_condattr_init(pthread_condattr_t *attr)
{
  return (attr == NULL) ? EINVAL : 0;
}

int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return (attr == NULL) ? EINVAL : 0;
}

int
pthread_condattr_setpshared(pthread_condattr_t *attr,
			    int pshared)
{
  return (attr == NULL) ? EINVAL : ENOSYS;
}

int
pthread_condattr_getpshared(pthread_condattr_t *attr,
			    int *pshared)
{
  return (attr == NULL) ? EINVAL : ENOSYS;
}

int
pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *attr)
{
  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Initialize the count to 0. */
  cv->waiters_count = 0;

  /* Initialize the "mutex". */
  pthread_mutex_init(cv->waiters_count_lock);

  /* Create an auto-reset event. */
  cv->events[SIGNAL] = CreateEvent (NULL,     /* no security */
				    FALSE,    /* auto-reset event */
				    FALSE,    /* non-signaled initially */
				    NULL);    /* unnamed */

  /* Create a manual-reset event. */
  cv->events[BROADCAST] = CreateEvent (NULL,  /* no security */
				       TRUE,  /* manual-reset */
				       FALSE, /* non-signaled initially */
				       NULL); /* unnamed */

  return 0;
}

/* This is an internal routine that allows the functions `pthread_cond_wait' and
   `pthread_cond_timedwait' to share implementations.  The `abstime'
   parameter to this function is in millisecond units (or INFINITE). */

static int
cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex, DWORD abstime)
{
  int result, last_waiter;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Avoid race conditions. */
  EnterCriticalSection (&cv->waiters_count_lock);
  cv->waiters_count_++;
  LeaveCriticalSection (&cv->waiters_count_lock);

  /* It's okay to release the mutex here since Win32 manual-reset
     events maintain state when used with SetEvent().  This avoids the
     "lost wakeup" bug. */

  pthread_mutex_unlock(mutex);

  /* Wait for either event to become signaled due to
     pthread_cond_signal() being called or pthread_cond_broadcast()
     being called. */
 
  result = WaitForMultipleObjects (2, ev->events, FALSE, abstime);

  EnterCriticalSection (&cv->waiters_count_lock);
  cv->waiters_count--;
  last_waiter = cv->waiters_count == 0;
  LeaveCriticalSection (&cv->waiters_count_lock);

  /* Some thread called pthread_cond_broadcast(). */
  if ((result = WAIT_OBJECT_0 + BROADCAST) && last_waiter)
    {
      /* We're the last waiter to be notified, so reset the manual
	 event. */
      ResetEvent(cv->events[BROADCAST]);
    }

  /* Reacquire the mutex. */
  pthread_mutex_lock(mutex);

  return 0;
}

int
pthread_cond_wait(pthread_cond_t *cv,
		  pthread_mutex_t *mutex)
{
  return cond_wait(cv, mutex, INFINITE);
}

/* Assume that our configure script will test for the existence of
   `struct timespec' and define it according to POSIX if it isn't
   found.  This will enable people to use this implementation
   without necessarily needing Cygwin32. */

int
pthread_cond_timedwait(pthread_cond_t *cv, 
		       pthread_mutex_t *mutex,
		       const struct timespec *abstime)
{
  DWORD msecs;
  
  /* Calculate the number of milliseconds in abstime. */
  msecs = abstime->tv_sec * 1000;
  msecs += abstime->tv_nsec / 1000000;

  return cond_wait(cv, mutex, msecs);
}

int 
pthread_cond_broadcast (pthread_cond_t *cv)
{
  int have_waiters;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Avoid race conditions. */
  EnterCriticalSection (&cv->waiters_count_lock_);
  have_waiters = (cv->waiters_count > 0);
  LeaveCriticalSection (&cv->waiters_count_lock_);

  if (have_waiters) {
    SetEvent(cv->events[BROADCAST]);
  }

  return 0;
}

int 
pthread_cond_signal (pthread_cond_t *cv)
{
  int have_waiters;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Avoid race conditions. */
  EnterCriticalSection (&cv->waiters_count_lock);
  have_waiters = (cv->waiters_count > 0);
  LeaveCriticalSection (&cv->waiters_count_lock);

  if (have_waiters) {
    SetEvent(cv->events[SIGNAL]);
  }

  return 0;
}

int
pthread_cond_destroy(pthread_cond_t *cv)
{
  if (cv == NULL)
    {
	return EINVAL;
    }

  return pthread_mutex_destroy(cv->waiters_count_lock);
}

/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include <windows.h>
#include <process.h>
#include <string.h>

#include "pthread.h"
#include "implement.h"

unsigned
_pthread_start_call(void * us_arg)
{
  /* We're now in a running thread. Any local variables here are on
     this threads private stack so we're safe to leave data in them
     until we leave. */
  _pthread_threads_thread__t * us;
  _pthread_call_t * call;
  unsigned (*func)(void *);
  void * arg;
  unsigned ret;
  int from;

  us = (_pthread_threads_thread__t *) us_arg;

  /* FIXME: For now, if priority setting fails then at least ensure
     that our records reflect true reality. */
  if (SetThreadPriority((HANDLE) us->thread, us->attr.priority) == FALSE)
    {
      us->attr.priority = GetThreadPriority((HANDLE) us->thread);
    }

  func = us->call.routine;
  arg = us->call.arg;

  /* FIXME: Should we be using sigsetjmp() here instead. */
  from = setjmp(us->call.env);

  if (from == 0)
    {
      /* Normal return from setjmp(). */
      ret = (*func)(arg);

      _pthread_vacuum();

      /* Remove the thread entry on exit only if pthread_detach()
	 was called and there are no waiting joins. */

      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      if (us->detach == TRUE
	  && us->join_count == 0)
	{
	  _pthread_delete_thread_entry(us);
	}

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */
    }
  else
    {
      /* longjmp() teleported us here.
	 func() called pthread_exit() which called longjmp(). */
      _pthread_vacuum();

      /* Remove the thread entry on exit only if pthread_detach()
	 was called and there are no waiting joins. */

      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      if (us->detach == TRUE
	  && us->join_count == 0)
	{
	  _pthread_delete_thread_entry(us);
	}

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */

      ret = 0;
    }

  /* From Win32's point of view we always return naturally from our
     start routine and so it should clean up it's own thread residue. */
  return ret;
}

int
pthread_create(pthread_t *thread, 
	       const pthread_attr_t *attr,
	       void * (*start_routine) (void *), 
	       void * arg)
{
  HANDLE   handle = NULL;
  unsigned flags;
  void *   security = NULL;
  DWORD  threadID;
  pthread_attr_t * attr_copy;
  _pthread_threads_thread_t * us;
  /* Success unless otherwise set. */
  int ret;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  ret = _pthread_new_thread_entry((pthread_t) handle, us);

  pthread_mutex_lock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  if (ret == 0)
    {
      attr_copy = &(us->attr);

      /* Map given attributes otherwise just use default values. */
      if (attr != NULL) 
	{
	  if (attr_copy->stacksize == 0)
	    {
	      attr_copy->stacksize = PTHREAD_STACK_MIN;
	    }

	  attr_copy->detachedstate = attr->detachedstate;
	  attr_copy->priority = attr->priority;

#if HAVE_SIGSET_T
	  memcpy(&(attr_copy->sigmask), &(attr->sigmask), sizeof(sigset_t)); 
#endif /* HAVE_SIGSET_T */
	}

      /* Start running, not suspended. */
      flags = 0;

      handle = (HANDLE) _beginthreadex(security,
				       attr_copy->stacksize,
				       _pthread_start_call,
				       (void *) us,
				       flags,
				       &threadID);

      if (handle == NULL)
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
      *thread = (pthread_t) handle;
    }
  else
    {
      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      /* Remove the failed thread entry. */
      _pthread_delete_thread_entry(us);

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */
    }

  return ret;
}




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
     this thread's private stack so we're safe to leave data in them
     until we leave. */
  pthread_t us;
  _pthread_call_t * call;
  unsigned (*func)(void *);
  void * arg;
  unsigned ret;
  int from;

  us = (pthread_t) us_arg;

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
  pthread_t new_thread;
  /* Success unless otherwise set. */
  int ret;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  ret = _pthread_new_thread(&new_thread);

  pthread_mutex_lock(&_pthread_table_mutex);
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

	  attr_copy->detachedstate = attr->detachedstate;
	  attr_copy->priority = attr->priority;

#if HAVE_SIGSET_T
	  memcpy(&(attr_copy->sigmask), &(attr->sigmask), sizeof(sigset_t)); 
#endif /* HAVE_SIGSET_T */
	}

      /* We call a generic wrapper which then calls the start routine. */
      new_thread->call.routine = start_routine;
      new_thread->call.arg = arg;

      /* Start running, not suspended. */
      flags = 0;

      handle = (HANDLE) _beginthreadex(security,
				       attr_copy->stacksize,
				       _pthread_start_call,
				       (void *) new_thread,
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

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */
    }

  return ret;
}




/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

unsigned
_pthread_start_call(void * call)
{
  /* We're now in a running thread. Any local variables here are on
     this threads private stack so we're safe to leave data in them
     until we leave. */
  _pthread_call_t * this;
  unsigned (*func)(void *);
  void * arg;
  unsigned ret;

  this = (_pthread_call_t *) call;
  func = call->routine;
  arg = call->arg;

  ret = (*func)(arg);

  /* If we get to here then we're returning naturally and haven't
     been cancelled. We need to cleanup and remove the thread
     from the threads table. */
  _pthread_vacuum();

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
  _pthread_threads_thread_t * this;
  /* Success unless otherwise set. */
  int ret = 0;

  if (_pthread_new_thread_entry((pthread_t) handle, this) == 0)
    {
      attr_copy = &(this->attr);

      if (attr != NULL) 
	{
	  /* Map attributes. */
	  if (attr_copy->stacksize == 0)
	    {
	      attr_copy->stacksize = PTHREAD_STACK_MIN;
	    }

	  attr_copy->cancelability = attr->cancelability;
	  attr_copy->canceltype = attr->canceltype;
	  attr_copy->detached = attr->detached;
	  attr_copy->priority = attr->priority;
	}

      /* Start running, not suspended. */
      flags = 0;

      handle = (HANDLE) _beginthreadex(security,
				       attr_copy->stacksize,
				       _pthread_start_call,
				       (void *) &(this->call),
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
      /* Undo everything. */
      _pthread_delete_thread_entry(this);
    }

  return ret;
}

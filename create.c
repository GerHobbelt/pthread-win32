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
_pthread_start_call(void *this)
{
  /* We're now in a running thread. Any local variables here are on
     this threads private stack so we're safe to leave data in them
     until we leave. */
  unsigned (*func)(void *) = this->call.routine;
  void * arg = this->call.arg;
  unsigned ret;

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

  /* FIXME: This needs to be moved into process space. 
     Perhaps into a structure that contains all
     per thread info that is Win32 thread specific but
     not visible from the pthreads API, and
     accessible through HANDLE (or pthread_t).
   */
  SECURITY_ATTRIBUTES security_attr;
  DWORD  threadID;
  /* Success unless otherwise set. */
  int ret = 0;
  pthread_attr_t * attr_copy;
  _pthread_threads_thread_t * this;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);

  if (_pthread_new_thread_entry((pthread_t) handle, &this) == 0)
    {
      attr_copy = &(this->attr);

      if (attr != NULL) 
	{
	  /* Map attributes */
	  if (attr_copy->stacksize == 0)
	    {
	      attr_copy->stacksize = PTHREAD_STACK_MIN;
	    }

	  attr_copy->cancelability = attr->cancelability;
	}

      /* Start suspended and resume at the last moment to avoid
	 race conditions, ie. where a thread may enquire it's
	 attributes before we finish storing them away. */
      flags = 1;

      handle = (HANDLE) _beginthreadex(security,
				       attr_copy->stacksize,
				       _pthread_start_call,
				       (void *) this,
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

  /* Let others in as soon as possible. */
  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */

  if (ret == 0)
    {
      /* Let the caller know the thread handle. */
      *thread = (pthread_t) handle;

      /* POSIX threads are always running after creation. */
      ResumeThread(handle);
    }
  else
    {
      /* Undo everything. */
      _pthread_delete_thread_entry(this);
    }

  return ret;
}

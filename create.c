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
_pthread_start_call(void * thisarg)
{
  /* We're now in a running thread. Any local variables here are on
     this threads private stack so we're safe to leave data in them
     until we leave. */
  _pthread_threads_thread__t * this = thisarg;
  _pthread_call_t * call;
  unsigned (*func)(void *);
  void * arg;
  unsigned ret;
  int from;

  if (this->detached == PTHREAD_CREATE_DETACHED)
    {
      (void) CloseHandle(this->thread);
    }

  func = this->call.routine;
  arg = this->call.arg;

  /* FIXME: Should we be using sigsetjmp() here instead. */
  from = setjmp(this->call.env);

  if (from == 0)
    {
      ret = (*func)(arg);

      _pthread_vacuum();
    }
  else
    {
      /* func() called pthread_exit() which called longjmp(). */
      _pthread_vacuum();

      /* Never returns. */
      _endthreadex(0);
    }

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

#if HAVE_SIGSET_T
	  memcpy(attr_copy.sigmask, attr.sigmask, sizeof(sigset_t)); 
#endif /* HAVE_SIGSET_T */
	}

      /* Start running, not suspended. */
      flags = 0;

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

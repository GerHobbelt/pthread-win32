/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

/* Thread ID management.
   ---------------------

   We started by simply mapping the Win32 thread handle directly to
   pthread_t. However, in order to process pthread_join()'s, we need
   to be able to keep our POSIX thread ID (pthread_t) around after the
   Win32 thread has terminated. Win32 may reuse the Win32 handle during that
   time, which will conflict.

   The pthread_t value is now actually the pointer to a thread struct:

   typedef struct _pthread * pthread_t;

   which amongst other things stores the Win32 thread handle:

   struct _pthread {
     HANDLE  win32handle;
     int     ptstatus;
     ...
   };

   So now whereever we need to use the Win32 handle it can be accessed
   as:

   pthread_t T = pthread_this();
   HANDLE    H;

   H = T->win32handle;

   // or (which is NOT preferred, let the compiler optimise to this).

   H = (HANDLE) *T;


   POSIX Threads Table
   -------------------

   Having the thread ID as a pointer to the thread struct itself
   avoids the need to search the threads table in all but the initial
   occasion where we create the thread.

   Initially we used a hash function to select a free thread struct
   from the table, possibly needing a walk through the table if the
   hash collided with an already in-use thread.

   The scheme used now is more efficient and is done as follows:

   We use two tables and two counters:

   struct _pthread  _pthread_virgins[PTHREAD_THREADS_MAX];
   pthread_t        _pthread_reuse[PTHREAD_THREADS_MAX];

   int       _pthread_virgin_next = 0;
   int       _pthread_reuse_top = -1;

   The counter _pthread_virgin_next is an index into _pthread_virgins[],
   which can be thought of as a list, and _pthread_reuse_top is an
   index into _pthread_reuse[], which can be thought of as a LIFO stack.

   Once taken from _pthread_virgins[], used and freed threads are only
   ever pushed back onto _pthread_reuse[].

 */

int
_pthread_new_thread(pthread_t * thread)
{
  pthread_t new_thread;

  if (_pthread_reuse_top >= 0)
    {
      new_thread = _pthread_reuse[_pthread_reuse_top--];
    }
  else
    {
      if (_pthread_virgin_next < PTHREAD_THREADS_MAX)
	{
	  new_thread = (pthread_t) &_pthread_virgins[_pthread_virgin_next++];
	}
      else
	{
	  return EAGAIN;
	}
    }

  new_thread->win32handle = (HANDLE) NULL;
  new_thread->ptstatus = _PTHREAD_NEW;
  pthread_attr_init(&(new_thread->attr));
  new_thread->joinvalueptr = NULL;
  new_thread->cancelstate = PTHREAD_CANCEL_ENABLE;
  new_thread->canceltype = PTHREAD_CANCEL_DEFERRED;
  new_thread->cancel_pending = FALSE;
  new_thread->cleanupstack = NULL;
  new_thread->forkpreparestack = NULL;
  new_thread->forkparentstack = NULL;
  new_thread->forkchildstack = NULL;

  *thread = new_thread;
  _pthread_threads_count++;

  return 0;
}

int
_pthread_delete_thread(_pthread_t * thread)
{
  /* We don't check that the thread has been properly cleaned up, so
     it had better be done already. */

  /* Release any keys */

  _pthread_destructor_run_all();

  /* Remove the thread entry if necessary. */

  if (thread != NULL
      && thread->ptstatus == _PTHREAD_EXITED)
    {
      pthread_attr_destroy(&(thread->attr));
      thread->win32handle = (HANDLE) NULL;
      thread->ptstatus = _PTHREAD_REUSE;

      _pthread_reuse[++_pthread_reuse_top] = thread;
      _pthread_threads_count--;

      return 0;
    }

  return EINVAL;
}

/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include "pthread.h"
#include "implement.h"

/* Thread ID management.
   ---------------------

   We started by simply mapping the Win32 thread handle to directly to
   pthread_t. Then, is order to process pthread_join()'s, needed to be
   able to keep our POSIX thread ID (pthread_t) around after the Win32
   thread has terminated and possibly reused the Win32 handle.

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
   occation where we create the thread.

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

   The code for choosing a new (pthread_t) thread from the pool of
   free thread structs looks like:

   if (_pthread_reuse_top == -1)
     {
       if (_pthread_virgin_next >= PTHREAD_THREADS_MAX)
         {
	   return EAGAIN;
	 }
       else
         {
	   thread = _pthread_virgin[_pthread_virgin_next++];
	 }
     }
   else
     {
       thread = _pthread_reuse[_pthread_reuse_top--];
     }


   The code to free a thread is:

   _pthread_reuse[++_pthread_reuse_top] = thread;

 */

int
_pthread_new_thread(pthread_t * thread)
{
  pthread_t new;

  if (_pthread_reuse_top == -1)
    {
      if (_pthread_virgin_next >= PTHREAD_THREADS_MAX)
	{
	  return EAGAIN;
	}
      else
	{
	  new = _pthread_virgin[_pthread_virgin_next++];
	}
    }
  else
    {
      new = _pthread_reuse[_pthread_reuse_top--];
    }

  new->win32handle = NULL;
  new->ptstatus = _PTHREAD_NEW;
  pthread_attr_init(&(new->attr));
  new->joinvalueptr = NULL;
  new->cancelstate = PTHREAD_CANCEL_ENABLE;
  new->canceltype = PTHREAD_CANCEL_DEFERRED;
  new->cancel_pending = FALSE;
  new->cleanupstack = NULL;
  new->destructorstack = NULL;
  new->forkpreparestack = NULL;
  new->forkparentstack = NULL;
  new->forkchildstack = NULL;

  *thread = new;

  return 0;
}

_pthread_threads_thread *
_pthread_find_thread(pthread_t thread)
{
  /* Should no longer be needed */
}

int
_pthread_delete_thread(_pthread_t * thread)
{
  /* We don't check that the thread has been properly cleaned up, so
     it had better be done already. */

  /* Remove the thread entry if necessary. */

  if (thread != NULL
      && thread->ptstatus == _PTHREAD_EXITED)
    {
      pthread_attr_destroy(&(entry->attr));
      thread->win32handle = NULL;
      thread_ptstatus = _PTHREAD_REUSE;

      _pthread_reuse[++_pthread_reuse_top] = thread;
    }
  else
    {
      return EINVAL
    }
  return 0;
}

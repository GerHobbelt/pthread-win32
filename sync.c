/*
 * sync.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
 */

#include <errno.h>

/* POSIX STANDARD: A thread may pass a value pointer to some data via
   pthread_exit(). That pointer will be stored in a location supplied
   as an argument to pthread_join().

   IMPLEMENTATION: The value_ptr is stored in the thread entry. When
   pthread_join() wakes up after waiting, or immediately if the target
   thread has already terminated but is not detached, the value
   pointer from pthread_exit() will be copied to *value_ptr.

   If the target thread does not become detached in the mean time, all
   waiting joins on that thread will get the value pointer. The last
   waiting join will delete the target thread entry.

   ----

   POSIX STANDARD: The results of multiple simultaneous calls to
   pthread_join() specifying the same target thread are undefined.

   IMPLEMENTATION: Any such join that occurs before the first such
   join wakes up, or the thread is otherwise detached (by a call to
   pthread_detach), will return successfully with the value that was
   passed to pthread_exit(). After the last such join returns, the
   target thread will have be detached and it's entry removed from the
   thread table.
  
   Until the target thread entry is deleted it will be counted against
   {PTHREAD_COUNT_MAX}.

   ----

   ----

   POSIX STANDARD: It is unspecified whether a thread that has exited
   but remains unjoined counts against {PTHREAD_COUNT_MAX}.

   IMPLEMENTATION: A thread that has exited but remains unjoined will
   be counted against {PTHREAD_COUNT_MAX}. The first call to
   pthread_join() or pthread_detach() will remove the target thread's
   table entry and decrement the count.

   ---- */

#include <windows.h>
#include "pthread.h"
#include "implement.h"

int
pthread_join(pthread_t thread, void ** valueptr)
{
  int detachstate;

  /* First check if we are trying to join to ourselves. */
  if (thread == pthread_self())
    {
      return EDEADLK;
    }

  if (thread != NULL)
    {
      int ret;

      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      /* If the thread is in DETACHED state, then join will return
	 immediately. */

      if (pthread_attr_getdetachstate(&(thread->attr), &detachstate) != 0 
	  || detachstate == PTHREAD_CREATE_DETACHED)
	{
	  return EINVAL;
	}

      thread->join_count++;

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */

      /* CANCELATION POINT */
      pthread_testcancel();

      /* Wait on the kernel thread object. */
      switch (WaitForSingleObject(thread->win32handle, INFINITE))
	{
	case WAIT_FAILED:
	  /* The thread does not exist. */
	  return ESRCH;
	case WAIT_OBJECT_0:
	  /* The thread has finished. */
	  break;
	default:
	  /* This should never happen. */
	  break;
	}

      /* We know the target thread entry still exists at this point
	 because we incremented join_count above after checking. The
	 thread entry will not be removed until join_count == 0 again,
	 ie. when the last waiting join has passed through the
	 following critical section. */

      /* CRITICAL SECTION */
      pthread_mutex_lock(&_pthread_table_mutex);

      /* Collect the value pointer passed to pthread_exit().  If
	 another thread detaches our target thread while we're
	 waiting, then we report a deadlock as it likely that storage
	 pointed to by thread->joinvalueptr has been freed or
	 otherwise no longer valid. */

      if (pthread_attr_getdetachstate(&(thread->attr), &detachstate) != 0 
	  || detachstate == PTHREAD_CREATE_DETACHED)
	{
	  ret = EDEADLK;
	}
      else
	{
	  *valueptr = thread->joinvalueptr;
	  ret = 0;
	}

      thread->join_count--;

      /* If we're the last join to return then we are responsible for
	 removing the target thread's table entry. */
      if (thread->join_count == 0)
	{
	  ret = _pthread_delete_thread(thread);
	}

      pthread_mutex_lock(&_pthread_table_mutex);
      /* END CRITICAL SECTION */

      return ret;
    }

  /* Thread not found. */
  return ESRCH;
}

int
pthread_detach(pthread_t thread)
{
  int detachstate;
  int ret;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  if (thread == NULL)
    {
      ret = ESRCH;
    }
  else
    {
      /* Check that we can detach this thread. */
      if (pthread_attr_getdetachstate(&(thread->attr), &detachstate) != 0 
	  || detachstate == PTHREAD_CREATE_DETACHED)
	{
	  ret = EINVAL;
	}
      else
	{
	  /* This is all we do here - the rest is done either when the
	     thread exits or when pthread_join() exits. Once this is
	     set it will never be unset. */
	  pthread_attr_setdetachstate(&(thread->attr), 
					PTHREAD_CREATE_DETACHED);

	  ret = 0;
	}
    }

  pthread_mutex_unlock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  return ret;
}

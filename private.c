/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include "pthread.h"
#include "implement.h"

/* The threads table works as follows:
   hash into the table,
   if the thread in this slot doesn't match then start single
   stepping from there until we find it, or we hit an empty slot, or
   we end up where we started from.

   The scheme should have these characteristics:
   - if the thread handle is a sequence number then the hash will
     succeed first time every time,
   - if the thread handle is a pseudo randomish value (eg. a pointer)
     then the hash should succeed first time most times.
 */

int
_pthread_new_thread_entry(pthread_t thread, _pthread_threads_thread_t * entry)
{
  _pthread_threads_thread_t * new;

  if (_pthread_threads_count >= PTHREAD_THREADS_MAX)
    {
      return EAGAIN;
    }

  new = &_pthread_threads_table[_PTHREAD_HASH_INDEX(thread)];

  while (new->thread != NULL)
    {
      new++;

      if (new == &_pthread_threads_table[PTHREAD_THREADS_MAX])
	{
	  /* Wrap to the top of the table. */
	  new == _pthread_threads_table;
	}
    }

  if (new->thread != NULL)
    {
      /* INTERNAL ERROR: There should be at least one slot left. */
      return ESRCH;
    }
  else
    {
      new->thread = thread;
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
    }

  _pthread_threads_count++;
  entry = new;

  return 0;
}

_pthread_threads_thread *
_pthread_find_thread_entry(pthread_t thread)
{
  _pthread_threads_thread_t * entry;
  _pthread_threads_thread_t * start;

  start = entry = &_pthread_threads_table[_PTHREAD_HASH_INDEX(thread)];

  while (entry->thread != thread)
    {
      entry++;

      if (entry == &_pthread_threads_table[PTHREAD_THREADS_MAX])
	{
	  /* Wrap to top of table. */
	  entry = _pthread_threads_table;
	}
    }

  if (entry->thread == NULL || entry == start)
    {
      /* Failed to find the thread. */
      return NULL;
    }

  return entry;
}

void
_pthread_delete_thread_entry(_pthread_threads_thread_t * entry)
{
  /* We don't check that the thread has been properly cleaned up, so
     it had better be done already. */

  /* Remove the thread entry if necessary. */

  if (entry->thread != NULL)
    {
      pthread_attr_destroy(&(entry->attr));
      entry->thread = NULL;

      if (_pthread_threads_count > 0)
	{
	  _pthread_threads_count--;
	}
      else
	{
	  /* FIXME: INTERNAL ERROR: This should not happen. */
	}
    }
  else
    {
      /* FIXME: INTERNAL ERROR: This should not happen. */
    }
}

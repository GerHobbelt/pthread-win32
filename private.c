/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include <windows.h>
#include <process.h>
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
  _pthread_threads_thread_t * this;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);

  if (_pthread_threads_count >= PTHREAD_THREADS_MAX)
    {
      return EAGAIN;
    }

  this = &_pthread_threads_table[_PTHREAD_HASH_INDEX(thread)];

  while (this->thread != NULL)
    {
      this++;

      if (this == &_pthread_threads_table[PTHREAD_THREADS_MAX])
	{
	  /* Wrap to the top of the table. */
	  this == _pthread_threads_table;
	}
    }

  if (this->thread != NULL)
    {
      /* INTERNAL ERROR: There should be at least one slot left. */
      return ESRCH;
    }
  else
    {
      this->thread = thread;
      pthread_attr_init(&(this->attr));
      this->joinvalueptr = NULL;
      this->cleanupstack = NULL;
      this->destructorstack = NULL;
      this->forkpreparestack = NULL;
      this->forkparentstack = NULL;
      this->forkchildstack = NULL;
    }

  _pthread_threads_count++;
  entry = this;

  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */

  return 0;
}

_pthread_threads_thread *
_pthread_find_thread_entry(pthread_t thread)
{
  _pthread_threads_thread_t * this;
  _pthread_threads_thread_t * start;

  start = this = &_pthread_threads_table[_PTHREAD_HASH_INDEX(thread)];

  while (this->thread != thread)
    {
      this++;

      if (this == &_pthread_threads_table[PTHREAD_THREADS_MAX])
	{
	  /* Wrap to top of table. */
	  this = _pthread_threads_table;
	}
    }

  if (this->thread == NULL || this == start)
    {
      /* Failed to find the thread. */
      return -1;
    }

  return this;
}

void
_pthread_delete_thread_entry(_pthread_threads_thread_t * this)
{
  /* We don't check that the thread has been properly cleaned up, so
     it had better be done already. */
  _pthread_threads_thread_t * this;
  _pthread_threads_thread_t * entry;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);

  /* If this is not NULL then we are removing an entry for a
     failed thread start. If this is NULL we need to get this
     here within the critical section. */
  if (this == NULL)
    {
      this = _PTHREAD_THIS;
    }

  if (this->thread != NULL)
    {
      this->thread = NULL;

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

  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */
}

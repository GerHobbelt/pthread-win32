/*
 * tsd.c
 *
 * Description:
 * POSIX thread functions which implement thread-specific data (TSD).
 */
 
/*
 * Why we can't use Win32 TLS
 * --------------------------
 *
 * In a word: Destructors
 *
 * POSIX 1003.1 1996, Section 17 allows for optional destructor functions
 * to be associated with each key value.
 *
 * This is my (revised) understanding of how destructors work:
 *
 * A key is created by a single thread, which then provides in every
 * existing thread a TSD matching the same key, but initialised
 * to NULL. Each new thread will also get a matching key with value NULL.
 * The creating thread can optionally associate a function, called a
 * destructor, with the key.
 *
 * When each thread exits, it calls the destructor function, which
 * will then perform an action on that threads key value
 * only. (Previously I thought that only the key creating thread ran
 * the destructor on the key in all threads. That proposition is
 * sounding scarier by the minute.)
 *
 * SOME APPROACHES TO MANAGING TSD MEMORY
 *
 * We could simply allocate enough memory on process startup to hold
 * all possible data for all possible threads.
 *
 * We could allocate memory for just a table to hold a single pointer
 * for each of POSIX_THREAD_KEYS_MAX keys. pthread_key_create() could then
 * allocate space for POSIX_THREADS_MAX key values in one hit and store
 * the location of the array in the first table.
 * 
 * The standard also suggests that each thread might store key/value pairs
 * on its private stack. This seems like a good idea. I had concerns about
 * memory leaks and key re-use if a key was deleted, but the standard talks
 * at length on this and basically says it's up to the application to
 * make sure everything goes smoothly here, making sure that proper cleanup
 * is done before a key is deleted. (section B.17.1.3 in particular)
 *
 * One more thing to note: destructors must never be called on deleted keys.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
  pthread_key_t k;
  int ret = 0;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_tsd_mutex);

  if (_pthread_tsd_key_next >= PTHREAD_KEYS_MAX)
    ret = EAGAIN;

  /* FIXME: This needs to be implemented as a list plus a re-use stack as for
     thread IDs. _pthread_destructor_run_all() then needs to be changed
     to push keys onto the re-use stack.
   */
  k = _pthread_tsd_key_next++;

  _pthread_tsd_key_table[k].in_use = 0;
  _pthread_tsd_key_table[k].status = _PTHREAD_TSD_KEY_INUSE;
  _pthread_tsd_key_table[k].destructor = destructor;

  pthread_mutex_unlock(&_pthread_tsd_mutex);
  /* END CRITICAL SECTION */

  *key = k;

  return ret;
}

int
pthread_setspecific(pthread_key_t key, void *value)
{
  void ** keys;
  int inuse;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_tsd_mutex);

  inuse = (_pthread_tsd_key_table[key].status == _PTHREAD_TSD_KEY_INUSE);

  pthread_mutex_unlock(&_pthread_tsd_mutex);
  /* END CRITICAL SECTION */

  if (! inuse)
    return EINVAL;

  keys = (void **) TlsGetValue(_pthread_TSD_keys_TlsIndex);

  if (keys[key] != NULL)
    {
      if (value == NULL)
	{
	  /* Key is no longer in use by this thread. */
	  _pthread_tsd_key_table[key].in_use--;
	}
    }
  else
    {
      if (value != NULL)
	{
	  /* Key is now in use by this thread. */
	  _pthread_tsd_key_table[key].in_use++;
	}
    }

  keys[key] = value;

  return 0;
}

void *
pthread_getspecific(pthread_key_t key)
{
  void ** keys;
  int inuse;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_tsd_mutex);

  inuse = (_pthread_tsd_key_table[key].status == _PTHREAD_TSD_KEY_INUSE);

  pthread_mutex_unlock(&_pthread_tsd_mutex);
  /* END CRITICAL SECTION */

  if (! inuse)
    return (void *) NULL;

  keys = (void **) TlsGetValue(_pthread_TSD_keys_TlsIndex);
  return keys[key];
}

/*
  pthread_key_delete:

  ANSI/IEEE Std 1003.1, 1996 Edition

  Section 17.1.3.2

  This function deletes a thread-specific data key previously returned by
  pthread_key_create(). The thread specific data values associated with
  "key" need not be NULL at the time pthread_key_delete() is called. It is
  the responsibility of the application to free any application storage
  or perform any cleanup actions for data structures related to the deleted
  key or associated thread-specific data in any threads; this cleanup
  can be done either before or after pthread_key_delete() is called. Any
  attempt to use "key" following the call to pthread_key_delete()
  results in undefined behaviour.

  The pthread_key_delete() function shall be callable from within
  destructor functions. No destructor functions shall be invoked by
  pthread_key_delete(). Any destructor function that may have been associated
  with "key" shall no longer be called upon thread exit.
 */

int
pthread_key_delete(pthread_key_t key)
{
  int ret = 0;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_tsd_mutex);

  if (_pthread_tsd_key_table[key].status != _PTHREAD_TSD_KEY_INUSE)
    {
      ret = EINVAL;
    }
  else
    {
      _pthread_tsd_key_table[key].status = _PTHREAD_TSD_KEY_DELETED;
      _pthread_tsd_key_table[key].destructor = NULL;
    }

  pthread_mutex_unlock(&_pthread_tsd_mutex);
  /* END CRITICAL SECTION */

  return ret;
}


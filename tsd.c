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

  if (_pthread_tsd_key_next >= PTHREAD_KEYS_MAX)
    return EAGAIN;

  k = _pthread_tsd_key_next++;

  _pthread_tsd_key_table[k].in_use = _PTHREAD_TSD_KEY_INUSE;
  _pthread_tsd_key_table[k].destructor = destructor;

  *key = k;

  return 0;
}

int
pthread_setspecific(pthread_key_t key, void *value)
{
  void ** keys;

  if (_pthread_tsd_key_table[key].in_use != _PTHREAD_TSD_KEY_INUSE)
    return EINVAL;

  keys = (void **) TlsGetValue(_pthread_TSD_keys_TlsIndex);
  keys[key] = value;

  return 0;
}

void *
pthread_getspecific(pthread_key_t key)
{
  void ** keys;

  if (_pthread_tsd_key_table[key].in_use != _PTHREAD_TSD_KEY_INUSE)
    return EINVAL;

  keys = (void **) TlsGetValue(_pthread_TSD_keys_TlsIndex);
  return keys[key];
}

int
pthread_key_delete(pthread_key_t key)
{
  if (_pthread_tsd_key_table[key].in_use != _PTHREAD_TSD_KEY_INUSE)
    return EINVAL;

  _pthread_tsd_key_table[key].in_use = _PTHREAD_TSD_KEY_DELETED;
  _pthread_tsd_key_table[key].destructor = NULL;

  return 0;
}


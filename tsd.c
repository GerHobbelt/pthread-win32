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

#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
pthread_key_create (pthread_key_t * key, void (*destructor) (void *))
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function creates a thread-specific data key visible
      *      to all threads. All existing and new threads have a value
      *      NULL for key until set using pthread_setspecific. When any
      *      thread with a non-NULL value for key terminates, 'destructor'
      *      is called with key's current value for that thread.
      *
      * PARAMETERS
      *      key
      *              pointer to an instance of pthread_key_t
      *
      *
      * DESCRIPTION
      *      This function creates a thread-specific data key visible
      *      to all threads. All existing and new threads have a value
      *      NULL for key until set using pthread_setspecific. When any
      *      thread with a non-NULL value for key terminates, 'destructor'
      *      is called with key's current value for that thread.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EAGAIN          insufficient resources or PTHREAD_KEYS_MAX
      *                              exceeded,
      *              ENOMEM          insufficient memory to create the key,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  
  if ((*key = (pthread_key_t) calloc (1, sizeof (**key))) == NULL)
    {
      result = ENOMEM;
    }
  else if (((*key)->key = TlsAlloc ()) == TLS_OUT_OF_INDEXES)
    {
      /*
       * Create system key
       */
      result = EAGAIN;

      free (*key);
      *key = NULL;
    }
  else if (destructor != NULL)
    {
      /*
       * Have to manage associations between thread and key;
       * Therefore, need a lock that allows multiple threads
       * to gain exclusive access to the key->threads list
       */
      result = pthread_mutex_init (&((*key)->threadsLock), NULL);

      if (result != 0)
        {
          TlsFree ((*key)->key);

          free (*key);
          *key = NULL;
        }
      (*key)->destructor = destructor;
    }

  return (result);
}

int
pthread_key_delete (pthread_key_t key)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function deletes a thread-specific data key. This
      *      does not change the value of the thread spcific data key
      *      for any thread and does not run the key's destructor
      *      in any thread so it should be used with caution.
      *
      * PARAMETERS
      *      key
      *              pointer to an instance of pthread_key_t
      *
      *
      * DESCRIPTION
      *      This function deletes a thread-specific data key. This
      *      does not change the value of the thread spcific data key
      *      for any thread and does not run the key's destructor
      *      in any thread so it should be used with caution.
      *
      * RESULTS
      *              0               successfully deleted the key,
      *              EINVAL          key is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (key != NULL)
    {
      if (key->threads != NULL &&
          pthread_mutex_lock (&(key->threadsLock)) == 0)
        {
          /*
           * Run through all Thread<-->Key associations
           * for this key.
           * If the pthread_t still exits (ie the assoc->thread
           * is not NULL) then leave the assoc for the thread to
           * destroy.
           * Notes:
           *      If assoc->thread is NULL, then the associated thread
           *      is no longer referencing this assoc.
           *      The association is only referenced
           *      by this key and must be released; otherwise
           *      the assoc will be destroyed when the thread is destroyed.
           */
          ThreadKeyAssoc *assoc;

          assoc = (ThreadKeyAssoc *) key->threads;

          while (assoc != NULL)
            {
              if (pthread_mutex_lock (&(assoc->lock)) == 0)
                {
                  ThreadKeyAssoc *next;

                  assoc->key = NULL;
                  next = assoc->nextThread;
                  assoc->nextThread = NULL;

                  pthread_mutex_unlock (&(assoc->lock));

                  tkAssocDestroy (assoc);

                  assoc = next;
                }
            }
          pthread_mutex_unlock (&(key->threadsLock));
        }

      TlsFree (key->key);
      if (key->destructor != NULL)
        {
          pthread_mutex_destroy (&(key->threadsLock));
        }

#if defined( _DEBUG )
      memset ((char *) key, 0, sizeof (*key));
#endif
      free (key);
    }

  return (result);
}


int
pthread_setspecific (pthread_key_t key, const void *value)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function initializes an unnamed semaphore. the
      *      initial value of the semaphore is 'value'
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *
      * DESCRIPTION
      *      This function  initializes an unnamed semaphore. The
      *      initial value of the semaphore is set to 'value'.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSPC          a required resource has been exhausted,
      *              ENOSYS          semaphores are not supported,
      *              EPERM           the process lacks appropriate privilege
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  int result = 0;

  if (key != _pthread_selfThreadKey)
    {
      /*
       * Using pthread_self will implicitly create
       * an instance of pthread_t for the current
       * thread if one wasn't explicitly created
       */
      self = pthread_self ();
    }
  else
    {
      /*
       * Resolve catch-22 of registering thread with threadSelf
       * key
       */
      self = pthread_getspecific (_pthread_selfThreadKey);
      if (self == NULL)
        {
          self = (pthread_t) value;
        }
    }

  result = 0;

  if (key != NULL)
    {
      ThreadKeyAssoc *assoc;

      if (self != NULL &&
          key->destructor != NULL &&
          value != NULL)
        {
          /*
           * Only require associations if we have to
           * call user destroy routine.
           * Don't need to locate an existing association
           * when setting data to NULL for WIN32 since the
           * data is stored with the operating system; not
           * on the association; setting assoc to NULL short
           * circuits the search.
           */
          assoc = (ThreadKeyAssoc *) self->keys;
          /*
           * Locate existing association
           */
          while (assoc != NULL)
            {
              if (assoc->key == key)
                {
                  /*
                   * Association already exists
                   */
                  break;
                }
              assoc = assoc->nextKey;
            }

          /*
           * create an association if not found
           */
          result = (assoc == NULL)
            ? tkAssocCreate (&assoc, self, key)
            : 0;
        }
      else
        {
          result = 0;
        }

      if (result == 0)
        {
          TlsSetValue (key->key, (LPVOID) value);
        }
    }
  return (result);
}                               /* pthread_setspecific */


void *
pthread_getspecific (pthread_key_t key)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns the current value of key in the
      *      calling thread. If no value has been set for 'key' in 
      *      the thread, NULL is returned.
      *
      * PARAMETERS
      *      key
      *              an instance of pthread_key_t
      *
      *
      * DESCRIPTION
      *      This function returns the current value of key in the
      *      calling thread. If no value has been set for 'key' in 
      *      the thread, NULL is returned.
      *
      * RESULTS
      *              key value
      *
      * ------------------------------------------------------
      */
{
  return (TlsGetValue (key->key));
}

/* </JEB> */


#if 0 /* Pre Bossom */

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

  if (_pthread_key_reuse_top >= 0)
    {
      k = _pthread_key_reuse[_pthread_key_reuse_top--];
    }
  else
    {
      if (_pthread_key_virgin_next < PTHREAD_KEYS_MAX)
	{
	  k = _pthread_key_virgins[_pthread_key_virgin_next++];
	}
      else
	{
	  return EAGAIN;
	}
    }

  /* FIXME: This needs to be implemented as a list plus a re-use stack as for
     thread IDs. _pthread_destructor_run_all() then needs to be changed
     to push keys onto the re-use stack.
   */

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

#endif /* Pre Bossom */

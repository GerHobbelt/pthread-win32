/*
 * tsd.c
 *
 * Description:
 * POSIX thread functions which implement thread-specific data (TSD).
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include "pthread.h"
#include "implement.h"

/* TLS_OUT_OF_INDEXES not defined on WinCE */
#ifndef TLS_OUT_OF_INDEXES
#define TLS_OUT_OF_INDEXES 0xffffffff
#endif

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
  pthread_key_t newkey;

  if ((newkey = (pthread_key_t) calloc (1, sizeof (*newkey))) == NULL)
    {
      result = ENOMEM;
    }
  else if ((newkey->key = TlsAlloc()) == TLS_OUT_OF_INDEXES)
    {
      result = EAGAIN;

      free (newkey);
      newkey = NULL;
    }
  else if (destructor != NULL)
    {
      /*
       * Have to manage associations between thread and key;
       * Therefore, need a lock that allows multiple threads
       * to gain exclusive access to the key->threads list.
       *
       * The mutex will only be created when it is first locked.
       */
      newkey->threadsLock = PTHREAD_MUTEX_INITIALIZER;
      newkey->destructor = destructor;
    }

  *key = newkey;

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
          key->destructor != NULL &&
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

                  ptw32_tkAssocDestroy (assoc);

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
      *      This function sets the value of the thread specific
      *	     key in the calling thread.
      *
      * PARAMETERS
      *      key
      *              an instance of pthread_key_t
      *	     value
      *		     the value to set key to
      *
      *
      * DESCRIPTION
      *      This function sets the value of the thread specific
      *      key in the calling thread.
      *
      * RESULTS
      *              0               successfully set value
      *              EAGAIN          could not set value
      *              ENOENT          SERIOUS!!
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  int result = 0;

  if (key != ptw32_selfThreadKey)
    {
      /*
       * Using pthread_self will implicitly create
       * an instance of pthread_t for the current
       * thread if one wasn't explicitly created
       */
      self = pthread_self ();
      if (self == NULL)
        {
          return ENOENT;
        }
    }
  else
    {
      /*
       * Resolve catch-22 of registering thread with threadSelf
       * key
       */
      self = (pthread_t) pthread_getspecific (ptw32_selfThreadKey);
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
	  if (assoc == NULL)
	    {
	      result = ptw32_tkAssocCreate (&assoc, self, key);
	    }
        }

      if (result == 0)
        {
          if ( ! TlsSetValue (key->key, (LPVOID) value))
	    {
	      result = EAGAIN;
	    }
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


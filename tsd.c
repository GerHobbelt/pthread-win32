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
 * to be associated with each key value. The destructors are called from
 * the creating thread, which means that the calling thread must have access
 * to the TSD keys of all active threads.
 *
 * If we use Win32 TLS then this is not possible since Tls*Value()
 * functions don't allow us to access other than our own [thread's] key.
 *
 * As a result, these routines need to be redesigned.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
  DWORD index;

  index = TlsAlloc();
  if (index == 0xFFFFFFFF)
    {
      return EAGAIN;
    }

  /* Only modify the `key' parameter if allocation was successful. */
  *key = index;

  if (destructor != NULL)
    {
      return (_pthread_destructor_push(destructor, *key));
    }

  return 0;
}

int
pthread_setspecific(pthread_key_t key, void *value)
{
  return (TlsSetValue(key, value) == FALSE) ? EINVAL : 0;
}

void *
pthread_getspecific(pthread_key_t key)
{
  return TlsGetValue(key);
}

int
pthread_key_delete(pthread_key_t key)
{
  /* Remove this key's destructors. */
  _pthread_destructor_pop(key);

  return (TlsFree(key) == FALSE) ? EINVAL : 0;
}

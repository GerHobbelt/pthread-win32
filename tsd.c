/*
 * tsd.c
 *
 * Description:
 * POSIX thread functions which implement thread-specific data (TSD).
 */

#include "pthread.h"

int
pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
  DWORD index;

  /* FIXME: the destructor function is ignored for now.  This needs to
     be managed via the same cleanup handler mechanism as user-define
     cleanup handlers during a thread exit. */

  if (destructor != NULL)
    {
      return EINVAL;
    }

  index = TlsAlloc();
  if (index == 0xFFFFFFFF)
    {
      return EAGAIN;
    }

  /* Only modify the `key' parameter if allocation was successful. */
  *key = index;

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
  /* FIXME: this code assumes that TlsGetValue returns NULL if the key
     is not present in the TLS structure.  Confirm. */

  return TlsGetValue(key);
}

int
pthread_key_delete(pthread_key_t key)
{
  return (TlsFree(key) == FALSE) ? EINVAL : 0;
}
  

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

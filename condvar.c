/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 */

#include "pthread.h"

int
pthread_condattr_init(pthread_condattr_t *attr)
{
  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  attr->ptr = malloc(sizeof(_pthread_condattr_t));
  if (attr->ptr == NULL)
    {
      return ENOMEM;
    }

  /* FIXME: fill out the structure with default values. */
  return 0;
}

int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }
  
  free(attr->ptr);
  return 0;
}

int
pthread_condattr_setpshared(pthread_condattr_t *attr,
			    int pshared)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  (_pthread_condattr_t *) (attr->ptr)->pshared = pshared;
  return 0;
}

int
pthread_condattr_getpshared(pthread_condattr_t *attr,
			    int *pshared)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  *pshared = (_pthread_condattr_t *) (attr->ptr)->pshared;
  return 0;
}

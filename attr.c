/*
 * attr.c
 *
 * Description:
 * This translation unit implements operations on thread attribute objects.
 */

#include "pthread.h"
#include "implement.h"

int
pthread_attr_setstacksize(pthread_attr_t *attr,
			  size_t stacksize)
{
  /* Verify that the stack size is within range. */
  if (stacksize < PTHREAD_STACK_MIN)
    {
      return EINVAL;
    }

  if (attr == NULL)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  attr->stacksize = stacksize;
  return 0;
}

int
pthread_attr_getstacksize(const pthread_attr_t *attr,
			  size_t *stacksize)
{
  if (attr == NULL)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  *stacksize = attr->stacksize;
  return 0;
}

int
pthread_attr_setstackaddr(pthread_attr_t *attr,
			  void *stackaddr)
{
  if (attr == NULL)
    {
      return EINVAL;
    }

  /* FIXME: it does not look like Win32 permits this. */
  return ENOSYS;
}

int
pthread_attr_getstackaddr(const pthread_attr_t *attr,
			  void **stackaddr)
{
  if (attr == NULL)
    {
      return EINVAL;
    }
  
  /* FIXME: it does not look like Win32 permits this. */
  return ENOSYS;
}

int
pthread_attr_init(pthread_attr_t *attr)
{
  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  /* FIXME: Fill out the structure with default values. */
  attr->stacksize = 0;

  return 0;
}

int
pthread_attr_destroy(pthread_attr_t *attr)
{
  if (attr == NULL)
    {
      return EINVAL;
    }
 
  /* Nothing to do. */
  return 0;
}

/*
 * attr.c
 *
 * Description:
 * This translation unit implements operations on thread attribute objects.
 */

#include "pthread.h"
#include "implement.h"

static int
is_attr(pthread_attr_t *attr)
{
  /* Return 0 if the attr object is valid, 1 otherwise. */

  return (attr == NULL || attr->valid != _PTHREAD_ATTR_VALID);
}

#ifdef _POSIX_THREAD_ATTR_STACKSIZE

int
pthread_attr_setstacksize(pthread_attr_t *attr,
			  size_t stacksize)
{
  /* Verify that the stack size is within range. */
  if (stacksize < PTHREAD_STACK_MIN)
    {
      return EINVAL;
    }

  if (is_attr(attr) != 0)
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
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  *stacksize = attr->stacksize;
  return 0;
}

#endif /* _POSIX_THREAD_ATTR_STACKSIZE */

#ifdef _POSIX_THREAD_ATTR_STACKADDR

int
pthread_attr_setstackaddr(pthread_attr_t *attr,
			  void *stackaddr)
{
  if (is_attr(attr) != 0)
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
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }
  
  /* FIXME: it does not look like Win32 permits this. */
  return ENOSYS;
}

#endif /* _POSIX_THREAD_ATTR_STACKADDR */


int
pthread_attr_init(pthread_attr_t *attr)
{
  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

#ifdef _POSIX_THREAD_ATTR_STACKSIZE
  attr->stacksize = PTHREAD_STACK_MIN;
#endif

  attr->cancelability = _PTHREAD_CANCEL_DEFAULTS;
  attr->valid = 0;

  return 0;
}

int
pthread_attr_destroy(pthread_attr_t *attr)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Set the attribute object to a specific invalid value. */
  attr->valid = _PTHREAD_ATTR_INVALID;

  return 0;
}

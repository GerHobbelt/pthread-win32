/*
 * attr.c
 *
 * Description:
 * This translation unit implements operations on thread attribute objects.
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

/* ignore warning "unreferenced formal parameter" */
#ifdef _MSC_VER
#pragma warning( disable : 4100 )
#endif

#include "pthread.h"
#include "implement.h"

static INLINE int
is_attr(const pthread_attr_t *attr)
{
  /* Return 0 if the attr object is valid, non-zero otherwise. */

  return (attr == NULL || 
	  *attr == NULL || 
	  (*attr)->valid != PTW32_ATTR_VALID);
}


int
pthread_attr_setstacksize(pthread_attr_t *attr,
			  size_t stacksize)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function specifies the size of the stack on
      *      which threads created with 'attr' will run.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      stacksize
      *              stack size, in bytes.
      *
      *
      * DESCRIPTION
      *      This function specifies the size of the stack on
      *      which threads created with 'attr' will run.
      *
      *      NOTES:
      *              1)      Function supported only if this macro is
      *                      defined:
      *
      *                              _POSIX_THREAD_ATTR_STACKSIZE
      *
      *              2)      Find the default first (using
      *                      pthread_attr_getstacksize), then increase
      *                      by multiplying.
      *
      *              3)      Only use if thread needs more than the
      *                      default.
      *
      * RESULTS
      *              0               successfully set stack size,
      *              EINVAL          'attr' is invalid or stacksize too
      *                              small or too big.
      *              ENOSYS          function not supported
      *
      * ------------------------------------------------------
      */
{
#ifdef _POSIX_THREAD_ATTR_STACKSIZE

  /*  Verify that the stack size is within range. */
  if (stacksize < PTHREAD_STACK_MIN)
    {
      return EINVAL;
    }

  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  (*attr)->stacksize = stacksize;
  return 0;

#else

  return ENOSYS;

#endif /* _POSIX_THREAD_ATTR_STACKSIZE */

}

int
pthread_attr_getstacksize(const pthread_attr_t *attr,
			  size_t *stacksize)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function determines the size of the stack on
      *      which threads created with 'attr' will run.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      stacksize
      *              pointer to size_t into which is returned the
      *              stack size, in bytes.
      *
      *
      * DESCRIPTION
      *      This function determines the size of the stack on
      *      which threads created with 'attr' will run.
      *
      *      NOTES:
      *              1)      Function supported only if this macro is
      *                      defined:
      *
      *                              _POSIX_THREAD_ATTR_STACKSIZE
      *
      *              2)      Use on newly created attributes object to
      *                      find the default stack size.
      *
      * RESULTS
      *              0               successfully retrieved stack size,
      *              EINVAL          'attr' is invalid
      *              ENOSYS          function not supported
      *
      * ------------------------------------------------------
      */
{
#ifdef _POSIX_THREAD_ATTR_STACKSIZE

  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  *stacksize = (*attr)->stacksize;
  return 0;

#else

  return ENOSYS;

#endif /* _POSIX_THREAD_ATTR_STACKSIZE */

}


int
pthread_attr_setstackaddr(pthread_attr_t *attr,
			  void *stackaddr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Threads created with 'attr' will run on the stack
      *      starting at 'stackaddr'.
      *      Stack must be at least PTHREAD_STACK_MIN bytes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      stacksize
      *              stack size, in bytes.
      *
      *
      * DESCRIPTION
      *      Threads created with 'attr' will run on the stack
      *      starting at 'stackaddr'.
      *      Stack must be at least PTHREAD_STACK_MIN bytes.
      *
      *      NOTES:
      *              1)      Function supported only if this macro is
      *                      defined:
      *
      *                              _POSIX_THREAD_ATTR_STACKADDR
      *
      *              2)      Create only one thread for each stack
      *                      address..
      *
      *              3)      Ensure that stackaddr is aligned.
      *
      * RESULTS
      *              0               successfully set stack address,
      *              EINVAL          'attr' is invalid
      *              ENOSYS          function not supported
      *
      * ------------------------------------------------------
      */
{
#if defined( _POSIX_THREAD_ATTR_STACKADDR )

  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  (*attr)->stackaddr = stackaddr;
  return 0;

#else

  return ENOSYS;

#endif /* _POSIX_THREAD_ATTR_STACKADDR */
}

int
pthread_attr_getstackaddr(const pthread_attr_t *attr,
			  void **stackaddr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function determines the address of the stack
      *      on which threads created with 'attr' will run.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      stackaddr
      *              pointer into which is returned the stack address.
      *
      *
      * DESCRIPTION
      *      This function determines the address of the stack
      *      on which threads created with 'attr' will run.
      *
      *      NOTES:
      *              1)      Function supported only if this macro is
      *                      defined:
      *
      *                              _POSIX_THREAD_ATTR_STACKADDR
      *
      *              2)      Create only one thread for each stack
      *                      address..
      *
      * RESULTS
      *              0               successfully retreived stack address,
      *              EINVAL          'attr' is invalid
      *              ENOSYS          function not supported
      *
      * ------------------------------------------------------
      */
{
#if defined( _POSIX_THREAD_ATTR_STACKADDR )
  
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  *stackaddr = (*attr)->stackaddr;
  return 0;

#else

  return ENOSYS;

#endif /* _POSIX_THREAD_ATTR_STACKADDR */
}


int
pthread_attr_init(pthread_attr_t *attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a thread attributes object with default
      *      attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *
      * DESCRIPTION
      *      Initializes a thread attributes object with default
      *      attributes.
      *
      *      NOTES:
      *              1)      Used to define thread attributes
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_attr_t attr_result;

  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  attr_result = (pthread_attr_t) malloc (sizeof (*attr_result));

  if (attr_result == NULL)
    {
      return ENOMEM;
    }

#ifdef _POSIX_THREAD_ATTR_STACKSIZE
  /*
   * Default to zero size. Unless changed explicitly this
   * will allow Win32 to set the size to that of the
   * main thread.
   */
  attr_result->stacksize = 0;
#endif

#ifdef _POSIX_THREAD_ATTR_STACKADDR
  /* FIXME: Set this to something sensible when we support it. */
  attr_result->stackaddr = NULL;
#endif

  attr_result->detachstate = PTHREAD_CREATE_JOINABLE;

#if HAVE_SIGSET_T
  memset(&(attr_result->sigmask), 0, sizeof(sigset_t));
#endif /* HAVE_SIGSET_T */

  /*
   * Win32 sets new threads to THREAD_PRIORITY_NORMAL and
   * not to that of the parent thread. We choose to default to
   * this arrangement.
   */
  attr_result->param.sched_priority = THREAD_PRIORITY_NORMAL;
  attr_result->inheritsched = PTHREAD_EXPLICIT_SCHED;

  attr_result->valid = PTW32_ATTR_VALID;

  *attr = attr_result;

  return 0;
}

int
pthread_attr_destroy(pthread_attr_t *attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Destroys a thread attributes object.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *
      * DESCRIPTION
      *      Destroys a thread attributes object.
      *
      *      NOTES:
      *              1)      Does not affect threads created with 'attr'.
      *
      * RESULTS
      *              0               successfully destroyed attr,
      *              EINVAL          'attr' is invalid.
      *
      * ------------------------------------------------------
      */
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /*
   * Set the attribute object to a specific invalid value.
   */
  (*attr)->valid = 0;
  free (*attr);
  *attr = NULL;

  return 0;
}

int
pthread_attr_getdetachstate(const pthread_attr_t *attr,
			    int *detachstate)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function determines whether threads created with
      *      'attr' will run detached.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      detachstate
      *              pointer to an integer into which is returned one
      *              of:
      *
      *              PTHREAD_CREATE_JOINABLE
      *                              Thread ID is valid, must be joined
      *
      *              PTHREAD_CREATE_DETACHED
      *                              Thread ID is invalid, cannot be joined,
      *                              canceled, or modified
      *
      *
      * DESCRIPTION
      *      This function determines whether threads created with
      *      'attr' will run detached.
      *
      *      NOTES:
      *              1)      You cannot join or cancel detached threads.
      *
      * RESULTS
      *              0               successfully retrieved detach state,
      *              EINVAL          'attr' is invalid
      *
      * ------------------------------------------------------
      */
{
  if (is_attr(attr) != 0 || detachstate == NULL)
    {
      *detachstate = PTHREAD_CREATE_DETACHED;
      return EINVAL;
    }

  *detachstate = (*attr)->detachstate;
  return 0;
}

int
pthread_attr_setdetachstate(pthread_attr_t *attr,
			    int detachstate)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function specifies whether threads created with
      *      'attr' will run detached.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_attr_t
      *
      *      detachstate
      *              an integer containing one of:
      *
      *              PTHREAD_CREATE_JOINABLE
      *                              Thread ID is valid, must be joined
      *
      *              PTHREAD_CREATE_DETACHED
      *                              Thread ID is invalid, cannot be joined,
      *                              canceled, or modified
      *
      *
      * DESCRIPTION
      *      This function specifies whether threads created with
      *      'attr' will run detached.
      *
      *      NOTES:
      *              1)      You cannot join or cancel detached threads.
      *
      * RESULTS
      *              0               successfully set detach state,
      *              EINVAL          'attr' or 'detachstate' is invalid
      *
      * ------------------------------------------------------
      */
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  if (detachstate != PTHREAD_CREATE_JOINABLE &&
      detachstate != PTHREAD_CREATE_DETACHED)
    {
      return EINVAL;
    }
  
  (*attr)->detachstate = detachstate;
  return 0;
}


int
pthread_attr_setscope(pthread_attr_t *attr, int contentionscope)
{
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
  if (contentionscope != PTHREAD_SCOPE_SYSTEM)
    {
      return ENOTSUP;
    }
 
  return 0;
#else
  return ENOSYS;
#endif
}
 
 
int
pthread_attr_getscope(const pthread_attr_t *attr, int *contentionscope)
{
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
  *contentionscope = PTHREAD_SCOPE_SYSTEM;
  return 0;
#else
  return ENOSYS;
#endif
}


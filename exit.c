/*
 * exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
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

void
pthread_exit (void *value_ptr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *
      * PARAMETERS
      *      value_ptr
      *              a generic data value (i.e. not the address of a value)
      *
      *
      * DESCRIPTION
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *      NOTE: thread should be joinable.
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;

  /* If the current thread is implicit it was not started through
     pthread_create(), therefore we cleanup and end the thread
     here. Otherwise we raise an exception to unwind the exception
     stack. The exception will be caught by _pthread_threadStart(),
     which will cleanup and end the thread for us.
   */

  self = (pthread_t) pthread_getspecific (_pthread_selfThreadKey);

  if (self == NULL || self->implicit)
    {
      _pthread_callUserDestroyRoutines(self);

      _endthreadex ((unsigned) value_ptr);
      
      /* Never reached */
    }
  else
    {
#ifdef _MSC_VER
 
      DWORD exceptionInformation[3];

      exceptionInformation[0] = (DWORD) (_PTHREAD_EPS_EXIT);
      exceptionInformation[1] = (DWORD) (value_ptr);
      exceptionInformation[2] = (DWORD) (0);

      RaiseException (
		      EXCEPTION_PTHREAD_SERVICES,
		      0,
		      3,
		      exceptionInformation);

#else /* ! _MSC_VER */

#ifdef __cplusplus

      self->exceptionInformation = value_ptr;
      throw Pthread_exception_exit();

#else /* ! __cplusplus */

      (void) pthread_pop_cleanup( 1 );

      _pthread_callUserDestroyRoutines(self);

      _endthreadex ((unsigned) value_ptr);

#endif /* __cplusplus */

#endif /* _MSC_VER */

    }
    
  /* Never reached. */

}

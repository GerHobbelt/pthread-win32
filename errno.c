/*
 * errno.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
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

#if (! defined(HAVE_ERRNO)) && (! defined(_REENTRANT)) && (! defined(_MT))

#include "pthread.h"
#include "implement.h"

static int reallyBad    = ENOMEM;

/*
 * Re-entrant errno.
 *
 * Each thread has it's own errno variable in pthread_t.
 *
 * The benefit of using the pthread_t structure
 * instead of another TSD key is TSD keys are limited
 * on Win32 to 64 per process. Secondly, to implement
 * it properly without using pthread_t you'd need
 * to dynamically allocate an int on starting the thread
 * and store it manually into TLS and then ensure that you free
 * it on thread termination. We get all that for free
 * by simply storing the errno on the pthread_t structure.
 *
 * MSVC and Mingw32 already have there own thread-safe errno.
 *
 * #if defined( _REENTRANT ) || defined( _MT )
 * #define errno *_errno()
 * 
 * int *_errno( void );
 * #else
 * extern int errno;
 * #endif
 *
 */

int * _errno( void )
{
  pthread_t       self;
  int             *result;
        
  if( ( self = pthread_self() ) == NULL )
    {
      /*
       * Yikes! unable to allocate a thread!
       * Throw an exception? return an error?
       */
      result = &reallyBad;
    }
  else
    {
      result = &(self->ptErrno);
    }

  return( result );

} /* _errno */

#endif /* (! HAVE_ERRNO) || (!_REENTRANT && (!_MT || !_MD)) */

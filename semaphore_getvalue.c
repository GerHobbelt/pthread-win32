/*
 * -------------------------------------------------------------
 *
 * Module: semaphore_getvalue.c
 *
 * Purpose:
 *	Semaphores aren't actually part of the PThreads standard.
 *	They are defined by the POSIX Standard:
 *
 *		POSIX 1003.1b-1993	(POSIX.1b)
 *
 * -------------------------------------------------------------
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2002 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@ise.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "pthread.h"
#include "semaphore.h"
#include "implement.h"


int
sem_getvalue(sem_t * sem, int * sval)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function stores the current count value of the
      *      semaphore.
      * RESULTS
      *
      * Return value
      *
      *       0                  sval has been set.
      *      -1                  failed, error in errno
      *
      *  in global errno
      *
      *      EINVAL              'sem' is not a valid semaphore,
      *      ENOSYS              this function is not supported,
      *
      *
      * PARAMETERS
      *
      *      sem                 pointer to an instance of sem_t
      *
      *      sval                pointer to int.
      *
      * DESCRIPTION
      *      This function stores the current count value of the semaphore
      *      pointed to by sem in the int pointed to by sval.
      */
{
  int result = 0;
 
  if ( sem == NULL || *sem == NULL || sval == NULL )
    {
      result = EINVAL;
    }
  else
    {

#ifdef NEED_SEM
 
      result = ENOSYS;
 
#else
      long value = *sval;
 
      /* Note:
       *  The windows NT documentation says that the increment must be
       *  greater than zero, but it is set to zero here. If this works,
       *  the function will return true. If not, we can't do it this way
       *  so flag it as not implemented.
       */

      if ( ReleaseSemaphore( (*sem)->sem, 0L, &value) )
        {
          *sval = value;
        }
      else
        {
          result = ENOSYS;
        }
 
#endif
    }
 
 
  if ( result != 0 )
    {
      errno = result;
      return -1;
    }
 
  return 0;
 
}   /* sem_getvalue */

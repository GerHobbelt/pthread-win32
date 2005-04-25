/*
 * -------------------------------------------------------------
 *
 * Module: sem_destroy.c
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
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
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
sem_destroy (sem_t * sem)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function destroys an unnamed semaphore.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      * DESCRIPTION
      *      This function destroys an unnamed semaphore.
      *
      * RESULTS
      *              0               successfully destroyed semaphore,
      *              -1              failed, error in errno
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EBUSY           threads (or processes) are currently
      *                                      blocked on 'sem'
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  sem_t s = NULL;

  if (sem == NULL || *sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
      s = *sem;
      *sem = NULL;

      if ((result = pthread_mutex_trylock (&s->lock)) == 0)
        {
          if (s->value >= 0)
            {
              (void) pthread_mutex_unlock (&s->lock);

              if (!CloseHandle (s->sem))
	        {
	          *sem = s;
	          result = EINVAL;
	        }
              else if ((result = pthread_mutex_destroy (&s->lock)) != 0)
                {

#ifdef NEED_SEM
		  s->sem = CreateEvent (NULL,
					PTW32_FALSE,    /* manual reset is false */
					PTW32_FALSE,    /* initial state is unset */
					NULL);
#else
                  s->sem = CreateSemaphore (NULL,      /* Always NULL */
                                            (long) 0,  /* Force threads to wait */
                                            (long) _POSIX_SEM_VALUE_MAX,       /* Maximum value */
                                            NULL);     /* Name */
#endif

                  if (s->sem == 0)
                    {
                      /* We just have to pretend that we've destroyed the semaphore
                       * even though we're leaving a mutex around.
                       */
                      result = 0;
                    }
                  else
                    {
                      *sem = s;
                      if (result != EBUSY)
                        result = EINVAL;
                    }
                }
            }
          else
            {
              (void) pthread_mutex_unlock (&s->lock);
              result = EBUSY;
            }
        }
    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  free (s);

  return 0;

}				/* sem_destroy */

/*
 * pthread_mutex_destroy.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
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
#include "implement.h"


int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL
      || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * Check to see if we have something to delete.
   */
  if (*mutex != PTHREAD_MUTEX_INITIALIZER)
    {
      mx = *mutex;

      result = pthread_mutex_trylock(&mx);

      /*
       * The mutex type may not be RECURSIVE therefore trylock may return EBUSY if
       * we already own the mutex. Here we are assuming that it's OK to destroy
       * a mutex that we own and have locked recursively. Is this correct?
       *
       * For FAST mutexes we record the owner as ANONYMOUS for speed. In this
       * case we assume that the thread calling pthread_mutex_destroy() is the
       * owner, if the mutex is owned at all.
       */
      if (result == 0
	  || mx->ownerThread == (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS
	  || pthread_equal( mx->ownerThread, pthread_self() ) )
	{
	  /*
	   * FIXME!!!
	   * The mutex isn't held by another thread but we could still
	   * be too late invalidating the mutex below since another thread
	   * may already have entered mutex_lock and the check for a valid
	   * *mutex != NULL.
	   */
	  *mutex = NULL;

	  result = pthread_mutex_unlock(&mx);

	  if (result == 0)
	    {
	      (void) sem_destroy( &mx->wait_sema );
	      DeleteCriticalSection( &mx->wait_cs );
	      free(mx);
	    }
	  else
	    {
	      /*
	       * Restore the mutex before we return the error.
	       */
	      *mutex = mx;
	    }
	}
    }
  else
    {
      /*
       * See notes in ptw32_mutex_check_need_init() above also.
       */
      EnterCriticalSection(&ptw32_mutex_test_init_lock);

      /*
       * Check again.
       */
      if (*mutex == PTHREAD_MUTEX_INITIALIZER)
	{
	  /*
	   * This is all we need to do to destroy a statically
	   * initialised mutex that has not yet been used (initialised).
	   * If we get to here, another thread
	   * waiting to initialise this mutex will get an EINVAL.
	   */
	  *mutex = NULL;
	}
      else
	{
	  /*
	   * The mutex has been initialised while we were waiting
	   * so assume it's in use.
	   */
	  result = EBUSY;
	}

      LeaveCriticalSection(&ptw32_mutex_test_init_lock);
    }

  return(result);
}

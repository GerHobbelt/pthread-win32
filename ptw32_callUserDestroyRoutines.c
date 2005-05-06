/*
 * ptw32_callUserDestroyRoutines.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
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
#include "implement.h"

#ifdef __cplusplus
# if ! defined (_MSC_VER) && ! (defined(__GNUC__) && __GNUC__ < 3) && ! defined(__WATCOMC__)
using
  std::terminate;
# endif
#endif

void
ptw32_callUserDestroyRoutines (pthread_t thread)
     /*
      * -------------------------------------------------------------------
      * DOCPRIVATE
      *
      * This the routine runs through all thread keys and calls
      * the destroy routines on the user's data for the current thread.
      * It simulates the behaviour of POSIX Threads.
      *
      * PARAMETERS
      *              thread
      *                      an instance of pthread_t
      *
      * RETURNS
      *              N/A
      * -------------------------------------------------------------------
      */
{
  ThreadKeyAssoc * next;
  ThreadKeyAssoc * assoc;

  if (thread.p != NULL)
    {
      ptw32_thread_t * sp = (ptw32_thread_t *) thread.p;

      /*
       * Run through all Thread<-->Key associations
       * for the current thread.
       */
      assoc = next = (ThreadKeyAssoc *) sp->keys;

      while (assoc != NULL)
	{
	  pthread_key_t k;

	  if ((k = assoc->key) != NULL
	      && pthread_mutex_lock(&(k->keyLock)) == 0)
	    {
	      /*
	       * Key still active; pthread_key_delete
	       * will block on this same mutex before
	       * it can release actual key; therefore,
	       * key is valid and we can call the destroy
	       * routine;
	       */
	      void * value = NULL;

	      value = pthread_getspecific (k);
	      if (value != NULL && k->destructor != NULL)
		{

#ifdef __cplusplus

		  try
		    {
		      /*
		       * Run the caller's cleanup routine.
		       */
		      (*(k->destructor)) (value);
		    }
		  catch (...)
		    {
		      /*
		       * A system unexpected exception has occurred
		       * running the user's destructor.
		       * We get control back within this block in case
		       * the application has set up it's own terminate
		       * handler. Since we are leaving the thread we
		       * should not get any internal pthreads
		       * exceptions.
		       */
		      (void) pthread_mutex_unlock(&(k->keyLock));
		      terminate ();
		    }

#else /* __cplusplus */

		  /*
		   * Run the caller's cleanup routine.
		   */
		  (*(k->destructor)) (value);

#endif /* __cplusplus */
		}

	      /*
	       * Remove association from both the key and thread chains
	       */
	      (void) pthread_mutex_lock(&(sp->threadLock));
	      next = assoc->nextKey;
	      ptw32_tkAssocDestroy (assoc);
	      (void) pthread_mutex_unlock(&(sp->threadLock));

	      assoc = next;

	      (void) pthread_mutex_unlock(&(k->keyLock));
	    }
	}
    }

}				/* ptw32_callUserDestroyRoutines */

/*
 * ptw32_threadReuse.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"


/*
 * How it works:
 * A pthread_t is a struct (2x32 bit scalar types on x86, 2x64 bit on x86_64)
 * [FIXME: This is not true, x86_64 is 64 bit pointer and 32 bit counter. This
 * should be fixed in version 3.0.0]
 * which is normally passed/returned by value to/from pthreads routines.
 * Applications are therefore storing a copy of the struct as it is at that
 * time.
 *
 * The original pthread_t struct plus all copies of it contain the address of
 * the thread state struct __ptw32_thread_t_ (p), plus a reuse counter (x). Each
 * __ptw32_thread_t contains the original copy of it's pthread_t (ptHandle).
 * Once malloced, a __ptw32_thread_t_ struct is not freed until the process exits.
 *
 * The thread reuse stack is a simple LILO stack managed through a singly
 * linked list element in the __ptw32_thread_t.
 *
 * Each time a thread is destroyed, the __ptw32_thread_t address is pushed onto the
 * reuse stack after it's ptHandle's reuse counter has been incremented.
 *
 * The following can now be said from this:
 * - two pthread_t's refer to the same thread iff their __ptw32_thread_t reference
 * pointers are equal and their reuse counters are equal. That is,
 *
 *   equal = (a.p == b.p && a.x == b.x)
 *
 * - a pthread_t copy refers to a destroyed thread if the reuse counter in
 * the copy is not equal to (i.e less than) the reuse counter in the original.
 *
 *   threadDestroyed = (copy.x != ((__ptw32_thread_t *)copy.p)->ptHandle.x)
 *
 */

/*
 * Pop a clean pthread_t struct off the reuse stack.
 */
pthread_t
__ptw32_threadReusePop (void)
{
  pthread_t t = {NULL, 0};
  __ptw32_mcs_local_node_t node;

  __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &node);

  if  (__PTW32_THREAD_REUSE_EMPTY != __ptw32_threadReuseTop)
    {
      __ptw32_thread_t * tp;

      tp = __ptw32_threadReuseTop;

      __ptw32_threadReuseTop = tp->prevReuse;

      if  (__PTW32_THREAD_REUSE_EMPTY == __ptw32_threadReuseTop)
        {
          __ptw32_threadReuseBottom =  __PTW32_THREAD_REUSE_EMPTY;
        }

      tp->prevReuse = NULL;

      t = tp->ptHandle;
    }

  __ptw32_mcs_lock_release(&node);

  return t;

}

/*
 * Push a clean pthread_t struct onto the reuse stack.
 * Must be re-initialised when reused.
 * All object elements (mutexes, events etc) must have been either
 * destroyed before this, or never initialised.
 */
void
__ptw32_threadReusePush (pthread_t thread)
{
  __ptw32_thread_t * tp = (__ptw32_thread_t *) thread.p;
  pthread_t t;
  __ptw32_mcs_local_node_t node;

  __ptw32_mcs_lock_acquire(&__ptw32_thread_reuse_lock, &node);

  t = tp->ptHandle;
  memset(tp, 0, sizeof(__ptw32_thread_t));

  /* Must restore the original POSIX handle that we just wiped. */
  tp->ptHandle = t;

  /* Bump the reuse counter now */
#if defined (__PTW32_THREAD_ID_REUSE_INCREMENT)
  tp->ptHandle.x +=  __PTW32_THREAD_ID_REUSE_INCREMENT;
#else
  tp->ptHandle.x++;
#endif

  tp->state = PThreadStateReuse;

  tp->prevReuse =  __PTW32_THREAD_REUSE_EMPTY;

  if  (__PTW32_THREAD_REUSE_EMPTY != __ptw32_threadReuseBottom)
    {
      __ptw32_threadReuseBottom->prevReuse = tp;
    }
  else
    {
      __ptw32_threadReuseTop = tp;
    }

  __ptw32_threadReuseBottom = tp;

  __ptw32_mcs_lock_release(&node);
}

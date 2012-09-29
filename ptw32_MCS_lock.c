/*
 * pte_MCS_lock.c
 *
 * Description:
 * This translation unit implements queue-based locks.
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

/*
 * About MCS locks:
 *
 * MCS locks are queue-based locks, where the queue nodes are local to the
 * thread. The 'lock' is nothing more than a global pointer that points to
 * the last node in the queue, or is NULL if the queue is empty.
 * 
 * Originally designed for use as spin locks requiring no kernel resources
 * for synchronisation or blocking, the implementation below has adapted
 * the MCS spin lock for use as a general mutex that will suspend threads
 * when there is lock contention.
 *
 * Because the queue nodes are thread-local, most of the memory read/write
 * operations required to add or remove nodes from the queue do not trigger
 * cache-coherence updates.
 *
 * Like 'named' mutexes, MCS locks consume system resources transiently -
 * they are able to acquire and free resources automatically - but MCS
 * locks do not require any unique 'name' to identify the lock to all
 * threads using it.
 *
 * Usage of MCS locks:
 *
 * - you need a global pte_mcs_lock_t instance initialised to 0 or NULL.
 * - you need a local thread-scope pte_mcs_local_node_t instance, which
 *   may serve several different locks but you need at least one node for
 *   every lock held concurrently by a thread.
 *
 * E.g.:
 * 
 * pte_mcs_lock_t lock1 = 0;
 * pte_mcs_lock_t lock2 = 0;
 *
 * void *mythread(void *arg)
 * {
 *   pte_mcs_local_node_t node;
 *
 *   pte_mcs_acquire (&lock1, &node);
 *   pte_mcs_release (&node);
 *
 *   pte_mcs_acquire (&lock2, &node);
 *   pte_mcs_release (&node);
 *   {
 *      pte_mcs_local_node_t nodex;
 *
 *      pte_mcs_acquire (&lock1, &node);
 *      pte_mcs_acquire (&lock2, &nodex);
 *
 *      pte_mcs_release (&nodex);
 *      pte_mcs_release (&node);
 *   }
 *   return (void *)0;
 * }
 */

#include "implement.h"
#include "pthread.h"

/*
 * pte_mcs_flag_set -- notify another thread about an event.
 * 
 * Set event if an event handle has been stored in the flag, and
 * set flag to -1 otherwise. Note that -1 cannot be a valid handle value.
 */
INLINE void 
pte_mcs_flag_set (LONG * flag)
{
  HANDLE e = (HANDLE)PTE_INTERLOCKED_COMPARE_EXCHANGE(
						(PTE_INTERLOCKED_LPLONG)flag,
						(PTE_INTERLOCKED_LONG)-1,
						(PTE_INTERLOCKED_LONG)0);
  if ((HANDLE)0 != e)
    {
      /* another thread has already stored an event handle in the flag */
      SetEvent(e);
    }
}

/*
 * pte_mcs_flag_set -- wait for notification from another.
 * 
 * Store an event handle in the flag and wait on it if the flag has not been
 * set, and proceed without creating an event otherwise.
 */
INLINE void 
pte_mcs_flag_wait (LONG * flag)
{
  if (0 == InterlockedExchangeAdd((LPLONG)flag, 0)) /* MBR fence */
    {
      /* the flag is not set. create event. */

      HANDLE e = CreateEvent(NULL, PTE_FALSE, PTE_FALSE, NULL);

      if (0 == PTE_INTERLOCKED_COMPARE_EXCHANGE(
			                  (PTE_INTERLOCKED_LPLONG)flag,
			                  (PTE_INTERLOCKED_LONG)e,
			                  (PTE_INTERLOCKED_LONG)0))
	{
	  /* stored handle in the flag. wait on it now. */
	  WaitForSingleObject(e, INFINITE);
	}

      CloseHandle(e);
    }
}

/*
 * pte_mcs_lock_acquire -- acquire an MCS lock.
 * 
 * See: 
 * J. M. Mellor-Crummey and M. L. Scott.
 * Algorithms for Scalable Synchronization on Shared-Memory Multiprocessors.
 * ACM Transactions on Computer Systems, 9(1):21-65, Feb. 1991.
 */
INLINE void 
pte_mcs_lock_acquire (pte_mcs_lock_t * lock, pte_mcs_local_node_t * node)
{
  pte_mcs_local_node_t  *pred;
  
  node->lock = lock;
  node->nextFlag = 0;
  node->readyFlag = 0;
  node->next = 0; /* initially, no successor */
  
  /* queue for the lock */
  pred = (pte_mcs_local_node_t *)PTE_INTERLOCKED_EXCHANGE((LPLONG)lock,
						              (LONG)node);

  if (0 != pred)
    {
      /* the lock was not free. link behind predecessor. */
      pred->next = node;
      pte_mcs_flag_set(&pred->nextFlag);
      pte_mcs_flag_wait(&node->readyFlag);
    }
}

/*
 * pte_mcs_lock_release -- release an MCS lock.
 * 
 * See: 
 * J. M. Mellor-Crummey and M. L. Scott.
 * Algorithms for Scalable Synchronization on Shared-Memory Multiprocessors.
 * ACM Transactions on Computer Systems, 9(1):21-65, Feb. 1991.
 */
INLINE void 
pte_mcs_lock_release (pte_mcs_local_node_t * node)
{
  pte_mcs_lock_t *lock = node->lock;
  pte_mcs_local_node_t *next = (pte_mcs_local_node_t *)
    InterlockedExchangeAdd((LPLONG)&node->next, 0); /* MBR fence */

  if (0 == next)
    {
      /* no known successor */

      if (node == (pte_mcs_local_node_t *)
	  PTE_INTERLOCKED_COMPARE_EXCHANGE((PTE_INTERLOCKED_LPLONG)lock,
					     (PTE_INTERLOCKED_LONG)0,
					     (PTE_INTERLOCKED_LONG)node))
	{
	  /* no successor, lock is free now */
	  return;
	}
  
      /* wait for successor */
      pte_mcs_flag_wait(&node->nextFlag);
      next = (pte_mcs_local_node_t *)
	InterlockedExchangeAdd((LPLONG)&node->next, 0); /* MBR fence */
    }

  /* pass the lock */
  pte_mcs_flag_set(&next->readyFlag);
}

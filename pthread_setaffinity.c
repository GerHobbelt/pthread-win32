/*
 * pthread_setaffinity.c
 *
 * Description:
 * This translation unit implements thread cpu affinity setting.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
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
pthread_setaffinity_np(pthread_t thread, size_t cpusetsize,
                                  const cpu_set_t *cpuset)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *   The pthread_setaffinity_np() function sets the CPU affinity mask
      *   of the thread thread to the CPU set pointed to by cpuset.  If the
      *   call is successful, and the thread is not currently running on one
      *   of the CPUs in cpuset, then it is migrated to one of those CPUs.
      *
      * PARAMETERS
      *		thread
      *					The target thread
      *
      *		cpusetsize
      *					Ignored in pthreads4w.
      *					Usually set to sizeof(cpu_set_t)
      *
      *		cpuset
      *					The new cpu set mask.
      *
      *   				The set of CPUs on which the thread will actually run
      *   				is the intersection of the set specified in the cpuset
      *   				argument and the set of CPUs actually present for
      *   				the process.
      *
      * DESCRIPTION
      *   The pthread_setaffinity_np() function sets the CPU affinity mask
      *   of the thread thread to the CPU set pointed to by cpuset.  If the
      *   call is successful, and the thread is not currently running on one
      *   of the CPUs in cpuset, then it is migrated to one of those CPUs.
      *
      * RESULTS
      * 				0		Success
      * 				ESRCH	Thread does not exist
      * 				EFAULT	pcuset is NULL
      * 				EAGAIN	The thread affinity could not be set
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  ptw32_thread_t * tp;
  ptw32_mcs_local_node_t node;
  DWORD_PTR vProcessMask;
  DWORD_PTR vSystemMask;

  ptw32_mcs_lock_acquire (&ptw32_thread_reuse_lock, &node);

  tp = (ptw32_thread_t *) thread.p;

  if (NULL == tp
	  || thread.x != tp->ptHandle.x
	  || NULL == tp->threadH)
    {
	  result = ESRCH;
    }

  if (0 == result)
    {
	  if (cpuset)
	    {
		  if (GetProcessAffinityMask (OpenProcess (PROCESS_QUERY_INFORMATION,
				  	  	  	  	  	  	  	  	   PTW32_FALSE,
				  	  	  	  	  	  	  	  	   GetCurrentProcessId ()),
				  	  	  	  	  	  &vProcessMask,
				  	  	  	  	  	  &vSystemMask))
		  	{
			  /*
			   * Result is the intersection of available CPUs and the mask.
			   */
			  DWORD_PTR newMask = vProcessMask & *((PDWORD_PTR) cpuset);

			  if (newMask)
			    {
				  if (SetThreadAffinityMask (tp->threadH, newMask))
				    {
					  /*
					   * We record the intersection of the process affinity
					   * and the thread affinity cpusets so that
					   * pthread_getaffinity_np() returns the actual thread
					   * CPU set.
					   */
					  *(tp->cpuset) = newMask;
				    }
				  else
				  {
					result = EAGAIN;
				  }
			    }
			  else
			    {
				  result = EINVAL;
			    }
		  	}
		  else
		    {
			  result = EAGAIN;
		    }
	    }
	  else
	    {
		  result = EFAULT;
	    }
    }

  ptw32_mcs_lock_release (&node);

  return result;
}

int
pthread_getaffinity_np(pthread_t thread, size_t cpusetsize,
                                  const cpu_set_t *cpuset)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *   The pthread_getaffinity_np() function returns the CPU affinity mask
      *   of the thread thread in the CPU set pointed to by cpuset.
      *
      * PARAMETERS
      *		thread
      *					The target thread
      *
      *		cpusetsize
      *					Ignored in pthreads4w.
      *					Usually set to sizeof(cpu_set_t)
      *
      *		cpuset
      *					The location where the current cpu set
      *					will be returned.
      *
      *
      * DESCRIPTION
      *   The pthread_getaffinity_np() function returns the CPU affinity mask
      *   of the thread thread in the CPU set pointed to by cpuset.
      *
      * RESULTS
      * 				0		Success
      * 				ESRCH	thread does not exist
      * 				EFAULT	cpuset is NULL
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  ptw32_thread_t * tp;
  ptw32_mcs_local_node_t node;

  ptw32_mcs_lock_acquire(&ptw32_thread_reuse_lock, &node);

  tp = (ptw32_thread_t *) thread.p;

  if (NULL == tp
	  || thread.x != tp->ptHandle.x
	  || NULL == tp->threadH)
    {
	  result = ESRCH;
    }

  if (0 == result)
    {
	  if (cpuset)
	    {
		  *cpuset = *(tp->cpuset);
		}
	  else
	    {
		  result = EFAULT;
	    }
    }

  ptw32_mcs_lock_release(&node);

  return result;
}

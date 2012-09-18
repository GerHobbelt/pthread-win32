/*
 * sched_setaffinity.c
 *
 * Description:
 * POSIX scheduling functions that deal with CPU affinity.
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
#include "sched.h"

int
sched_setaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *mask)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      *	     If the process specified by pid is not currently running on
      *	     one of the CPUs specified in mask, then that process is
      *	     migrated to one of the CPUs specified in mask.
      *
      * PARAMETERS
      *      pid
      *      			Process ID
      *
      *      cpusetsize
      *      			Currently ignored in pthreads4w.
      *      			Usually set to sizeof(cpu_set_t)
      *
      *      mask
      *      			Pointer to the CPU mask to set (cpu_set_t).
      *
      * DESCRIPTION
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      *	     If the process specified by pid is not currently running on
      *	     one of the CPUs specified in mask, then that process is
      *	     migrated to one of the CPUs specified in mask.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EFAULT          'mask' is a NULL pointer.
      *              EINVAL          '*mask' contains no CPUs in the set
      *                              of available CPUs.
      *              EAGAIN          The system available CPUs could not
      *                              be obtained.
      *              EPERM           The process referred to by 'pid' is
      *                              not modifiable by us.
      *              ESRCH           The process referred to by 'pid' was
      *                              not found.
      *
      * ------------------------------------------------------
      */
{
#if ! defined(NEED_PROCESS_AFFINITY_MASK)

  DWORD_PTR vProcessMask;
  DWORD_PTR vSystemMask;
  HANDLE h;
  int targetPid = (int)(size_t) pid;
  int result = 0;

  if (NULL == mask)
    {
	  result = EFAULT;
    }
  else
    {
	  if (0 == pid)
	    {
		  int targetPid = (int) GetCurrentProcessId ();
	    }

	  h = OpenProcess (PROCESS_SET_INFORMATION, PTW32_FALSE, (DWORD) targetPid);

	  if (NULL == h)
	    {
		  result = (((0xFF & ERROR_ACCESS_DENIED) == GetLastError()) ? EPERM : ESRCH);
	    }
	  else
	    {
		  if (GetProcessAffinityMask (h, &vProcessMask, &vSystemMask))
		    {
			  /*
			   * Result is the intersection of available CPUs and the mask.
			   */
			  DWORD_PTR newMask = vSystemMask & *((PDWORD_PTR) mask);

			  if (newMask)
			    {
				  SetProcessAffinityMask(h, newMask);
			    }
			  else
			    {
				  /*
				   * Mask does not contain any CPUs currently available on the system.
				   */
				  result = EINVAL;
			    }
		    }
		  else
		    {
			  result = EAGAIN;
		    }
	    }
	  CloseHandle(h);
    }

  if (result != 0)
    {
	  PTW32_SET_ERRNO(result);
	  return -1;
    }
  else
#endif
    {
	  return 0;
    }
}


int
sched_getaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *mask)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Gets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      * PARAMETERS
      *      pid
      *      			Process ID
      *
      *      cpusetsize
      *      			Currently ignored in pthreads4w.
      *      			Usually set to sizeof(cpu_set_t)
      *
      *      mask
      *      			Pointer to the CPU mask to set (cpu_set_t).
      *
      * DESCRIPTION
      *      Sets the CPU affinity mask of the process whose ID is pid
      *	     to the value specified by mask.  If pid is zero, then the
      *	     calling process is used.  The argument cpusetsize is the
      *	     length (in bytes) of the data pointed to by mask.  Normally
      *	     this argument would be specified as sizeof(cpu_set_t).
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EFAULT          'mask' is a NULL pointer.
      *              EAGAIN          The system available CPUs could not
      *                              be obtained.
      *              EPERM           The process referred to by 'pid' is
      *                              not modifiable by us.
      *              ESRCH           The process referred to by 'pid' was
      *                              not found.
      *
      * ------------------------------------------------------
      */
{
  DWORD_PTR vProcessMask;
  DWORD_PTR vSystemMask;
  HANDLE h;
  int targetPid = (int)(size_t) pid;
  int result = 0;

  if (NULL == mask)
    {
	  result = EFAULT;
    }
  else
    {

#if ! defined(NEED_PROCESS_AFFINITY_MASK)

	  if (0 == pid)
	    {
		  int targetPid = (int) GetCurrentProcessId ();
	    }

	  h = OpenProcess (PROCESS_QUERY_INFORMATION, PTW32_FALSE, (DWORD) targetPid);

	  if (NULL == h)
	    {
		  result = (((0xFF & ERROR_ACCESS_DENIED) == GetLastError()) ? EPERM : ESRCH);
	    }
	  else
	    {
		  if (GetProcessAffinityMask (h, &vProcessMask, &vSystemMask))
		    {
			  *mask = (cpu_set_t)(size_t) *vProcessMask;
		    }
		  else
		    {
			  result = EAGAIN;
		    }
	    }
	  CloseHandle(h);

#else
	  *mask = 1;
#endif

    }

  if (result != 0)
    {
	  PTW32_SET_ERRNO(result);
	  return -1;
    }
  else
    {
	  return 0;
    }
}

/*
 * Support routines for cpu_set_t
 */
void CPU_ZERO (cpu_set_t *set)
{
  *set = (cpu_set_t)(size_t) 0;
}

void CPU_SET (int cpu, cpu_set_t *set)
{
  *set |= (cpu_set_t)(size_t) (1 << (cpu - 1));
}

void CPU_CLR (int cpu, cpu_set_t *set)
{
  *set &= (cpu_set_t)(size_t) (~(1 << (cpu - 1)));
}

int CPU_ISSET (int cpu, cpu_set_t *set)
{
  return ((*set & (1 << (cpu - 1))) != (cpu_set_t)(size_t) 0);
}

int CPU_COUNT (cpu_set_t *set)
{
  int bit, count = 0;
  cpu_set_t s = *set;

  for (bit = 1; bit < (1<<(sizeof(cpu_set_t)*8)); bit <<= 1)
    {
	  if (s & bit)
	    {
		  count++;
	    }
    }
  return count;
}

void CPU_AND (cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
  *destset = (cpu_set_t)((size_t)*srcset1 & (size_t)*srcset2);
}

void CPU_OR (cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
  *destset = (cpu_set_t)((size_t)*srcset1 | (size_t)*srcset2);
}

void CPU_XOR (cpu_set_t *destset, cpu_set_t *srcset1, cpu_set_t *srcset2)
{
  *destset = (cpu_set_t)((size_t)*srcset1 ^ (size_t)*srcset2);
}

int CPU_EQUAL (cpu_set_t *set1, cpu_set_t *set2)
{
	return ((size_t)*set1 == (size_t)*set2);
}

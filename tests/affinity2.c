/*
 * affinity2.c
 *
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
 *
 * --------------------------------------------------------------------------
 *
 * Have the process switch CPUs.
 *
 */

#include "test.h"

int
main()
{
  int result;
  cpu_set_t newmask;
  cpu_set_t mask = 0;
  cpu_set_t switchmask = 0x55555555;
  cpu_set_t flipmask   = 0xFFFFFFFF;

  assert(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == 0);

  if (!mask)
    {
	  result = -1;
    }
  else
    {
	  result = sched_setaffinity(0, sizeof(cpu_set_t), &mask);
	  if (result != 0)
		{
		  assert(errno != ESRCH);
		  assert(errno != EFAULT);
		  assert(errno != EPERM);
		  assert(errno != EINVAL);
		  assert(errno != EAGAIN);
		  assert(errno == ENOSYS);
		  assert(CPU_COUNT(&mask) == 1);
		}
	  else
	    {
		  if (CPU_COUNT(&mask) > 1)
		    {
			  printf("CPU mask Default = 0x%lx\n", (long unsigned int) mask);
			  CPU_AND(&newmask, &mask, &switchmask); /* Remove every other CPU */
			  assert(sched_setaffinity(0, sizeof(cpu_set_t), &newmask) == 0);
			  assert(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == 0);
			  printf("CPU mask New     = 0x%lx\n", (long unsigned int) mask);
			  CPU_XOR(&newmask, &mask, &flipmask);  /* Switch to all alternative CPUs */
			  assert(sched_setaffinity(0, sizeof(cpu_set_t), &newmask) == 0);
			  assert(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == 0);
			  printf("CPU mask Flipped = 0x%lx\n", (long unsigned int) mask);
			  assert(newmask != mask);
			}
	    }
    }

  return 0;
}

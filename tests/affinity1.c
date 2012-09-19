/*
 * affinity1.c
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
 * Basic test of sched_*affinity.
 * Basic test of CPU_*() support routines.
 *
 */

#include "test.h"

int
main()
{
  int result;
  cpu_set_t mask = 0;

  assert(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == 0);

  if (!mask)
    {
	  result = -1;
    }
  else
    {
	  cpu_set_t newmask = 0;
	  cpu_set_t src1mask = 0;
	  cpu_set_t src2mask = 0;
	  int cpu;

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

	  /*
	   * Use only the first 32 bits. Even though x64 will be twice that size
	   * this is only a test of bit manipulation routines.
	   */
	  for (cpu = 0; cpu < 32; cpu += 2)
		src1mask |= ((cpu_set_t)1 << cpu);		/* 0b01010101010101010101010101010101 */
	  for (cpu = 0; cpu < 16; cpu++)
		src2mask |= ((cpu_set_t)1 << cpu);		/* 0b00000000000000001111111111111111 */
	  for (cpu = 16; cpu < 32; cpu += 2)
		src2mask |= ((cpu_set_t)1 << cpu);		/* 0b01010101010101011111111111111111 */

	  assert(CPU_COUNT(&src1mask) == 16);
	  assert(CPU_COUNT(&src2mask) == 24);
	  assert((CPU_SET(0, &newmask), newmask == 1));
	  assert((CPU_SET(1, &newmask), newmask == 3));
	  assert((CPU_SET(3, &newmask), newmask == 11));
	  assert(CPU_ISSET(1, &newmask));
	  assert((CPU_CLR(1, &newmask), newmask == 9));
	  assert(!CPU_ISSET(1, &newmask));
	  assert((CPU_ZERO(&newmask), newmask == 0));
	  assert((CPU_OR(&newmask, &src1mask, &src2mask), newmask == src2mask));
	  assert((CPU_AND(&newmask, &src1mask, &src2mask), newmask == src1mask));
	  assert((CPU_XOR(&newmask, &src1mask, &src2mask), newmask == 43690 /* 0b00000000000000001010101010101010 */));
	  assert(CPU_EQUAL(&src1mask, &src1mask));
	  assert(!CPU_EQUAL(&src1mask, &src2mask));
    }

  return 0;
}

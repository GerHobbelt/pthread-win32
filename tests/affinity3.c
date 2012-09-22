/*
 * affinity3.c
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
 * Have the thread switch CPUs.
 *
 */

#include "test.h"

int
main()
{
  int result;
  unsigned int i;
  cpu_set_t newmask;
  cpu_set_t processCpus;
  cpu_set_t mask = 0;
  cpu_set_t switchmask = 0;
  cpu_set_t flipmask = 0;
  pthread_t self = pthread_self();

  assert(pthread_getaffinity_np(self, sizeof(cpu_set_t), &processCpus) == 0);
  printf("This thread has a starting affinity with %d CPUs\n", CPU_COUNT(&processCpus));
  assert(processCpus != 0);

  for (i = 0; i < sizeof(cpu_set_t); i++)
	switchmask |= ((cpu_set_t)0x55 << (8*i));	/* 0b01010101010101010101010101010101 */
  for (i = 0; i < sizeof(cpu_set_t); i++)
	flipmask |= ((cpu_set_t)0xff << (8*i));		/* 0b11111111111111111111111111111111 */

  result = pthread_setaffinity_np(self, sizeof(cpu_set_t), &processCpus);
  if (result != 0)
	{
	  assert(result != ESRCH);
	  assert(result != EFAULT);
	  assert(result != EPERM);
	  assert(result != EINVAL);
	  assert(result != EAGAIN);
	  assert(result == ENOSYS);
	  assert(CPU_COUNT(&mask) == 1);
	}
  else
	{
	  if (CPU_COUNT(&mask) > 1)
		{
		  CPU_AND(&newmask, &processCpus, &switchmask); /* Remove every other CPU */
		  assert(pthread_setaffinity_np(self, sizeof(cpu_set_t), &newmask) == 0);
		  assert(pthread_getaffinity_np(self, sizeof(cpu_set_t), &mask) == 0);
		  assert(CPU_EQUAL(&mask, &newmask));
		  CPU_XOR(&newmask, &mask, &flipmask);  /* Switch to all alternative CPUs */
		  assert(!CPU_EQUAL(&mask, &newmask));
		  assert(pthread_setaffinity_np(self, sizeof(cpu_set_t), &newmask) == 0);
		  assert(pthread_getaffinity_np(self, sizeof(cpu_set_t), &mask) == 0);
		  assert(CPU_EQUAL(&mask, &newmask));
		}
	}

  return 0;
}

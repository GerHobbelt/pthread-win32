/*
 * affinity5.c
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
 * Test thread CPU affinity inheritance.
 *
 */

#include "test.h"

void *
mythread(void * arg)
{
  HANDLE threadH = GetCurrentThread();
  cpu_set_t *parentCpus = (cpu_set_t*) arg;
  cpu_set_t threadCpus;
  DWORD_PTR vThreadMask;
  unsigned long int a, b; /* To stop GCC complaining about %lx args to printf */

  assert(pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &threadCpus) == 0);
  assert(CPU_EQUAL(parentCpus, &threadCpus));
  vThreadMask = SetThreadAffinityMask(threadH, (DWORD_PTR)threadCpus.cpuset /* Violating Opacity */);
  assert(vThreadMask != 0);
  assert(memcmp(&vThreadMask, &threadCpus, sizeof(DWORD_PTR)) == 0);
  printf("Parent/Thread CPU affinity = 0x%lx/0x%lx\n",
		  a = (unsigned long int)parentCpus->cpuset /* Violating Opacity */,
		  b = (unsigned long int)threadCpus.cpuset) /* Violating Opacity */;

  return (void*) 0;
}

int
main()
{
  unsigned int cpu;
  pthread_t tid;
  cpu_set_t threadCpus;
  DWORD_PTR vThreadMask;
  cpu_set_t keepCpus;
  pthread_t self = pthread_self();

  CPU_ZERO(&keepCpus);
  for (cpu = 1; cpu < sizeof(cpu_set_t)*8; cpu += 2)
    {
	  CPU_SET(cpu, &keepCpus);					/* 0b10101010101010101010101010101010 */
    }

  assert(pthread_getaffinity_np(self, sizeof(cpu_set_t), &threadCpus) == 0);
  if (CPU_COUNT(&threadCpus) > 1)
    {
	  assert(pthread_create(&tid, NULL, mythread, (void*)&threadCpus) == 0);
	  assert(pthread_join(tid, NULL) == 0);
	  CPU_AND(&threadCpus, &threadCpus, &keepCpus);
	  assert(pthread_setaffinity_np(self, sizeof(cpu_set_t), &threadCpus) == 0);
	  vThreadMask = SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)threadCpus.cpuset /* Violating Opacity */);
	  assert(vThreadMask != 0);
	  assert(memcmp(&vThreadMask, &threadCpus, sizeof(DWORD_PTR)) == 0);
	  assert(pthread_create(&tid, NULL, mythread, (void*)&threadCpus) == 0);
	  assert(pthread_join(tid, NULL) == 0);
    }

  return 0;
}

/* 
 * spin4.c
 *
 *
 * --------------------------------------------------------------------------
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
 *
 * --------------------------------------------------------------------------
 *
 * Declare a static spinlock object, lock it, spin on it, 
 * and then unlock it again.
 */

#include "test.h"
#include <sys/timeb.h>
 
pthread_spinlock_t lock = PTHREAD_SPINLOCK_INITIALIZER;
struct _timeb currSysTimeStart;
struct _timeb currSysTimeStop;

#define GetDurationMilliSecs(_TStart, _TStop) ((_TStop.time*1000+_TStop.millitm) \
                                               - (_TStart.time*1000+_TStart.millitm))

static int washere = 0;

void * func(void * arg)
{
  _ftime(&currSysTimeStart);
  assert(pthread_spin_lock(&lock) == 0);
  assert(pthread_spin_unlock(&lock) == 0);
  _ftime(&currSysTimeStop);
  washere = 1;

  return (void *) GetDurationMilliSecs(currSysTimeStart, currSysTimeStop);
}
 
int
main()
{
  long result = 0;
  int i;
  pthread_t t;
  int CPUs;

  if ((CPUs = pthread_num_processors_np()) == 1)
    {
      printf("Test not run - it requires multiple CPUs.\n");
	exit(0);
    }

  assert(pthread_spin_lock(&lock) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  /*
   * This should relinqish the CPU to the func thread enough times
   * to waste approximately 2000 millisecs only if the lock really
   * is spinning in the func thread (assuming 10 millisec CPU quantum).
   */
  for (i = 0; i < 200; i++)
    {
      sched_yield();
    }

  assert(pthread_spin_unlock(&lock) == 0);

  assert(pthread_join(t, (void **) &result) == 0);
  assert(result > 1000);

  assert(pthread_spin_destroy(&lock) == 0);

  assert(washere == 1);

  return 0;
}

/* 
 * spin4.c
 *
 * Declare a spinlock object, lock it, spin on it, 
 * and then unlock it again.
 *
 * For this to work on a single processor machine we have
 * to static initialise the spinlock. This bypasses the
 * check of the number of processors done by pthread_spin_init.
 * This is a non-portable side-effect of this implementation.
 */

#include "test.h"
#include <sys/timeb.h>
 
pthread_spinlock_t lock = PTHREADS_SPINLOCK_INITIALIZER;
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
  pthread_t t;

  assert(pthread_spin_lock(&lock) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  /*
   * This should relinqish the CPU to the func thread enough times
   * to waste approximately 2000 millisecs only if the lock really
   * is spinning in the func thread (assuming 10 millisec CPU quantum).
  for (i = 0; i < 200; i++)
    {
      sched_yield();
    }

  assert(pthread_spin_unlock(&lock) == 0);

  assert(pthread_join(t, (void *) &result) == 0);
  assert(result > 1000);

  assert(pthread_spin_destroy(&lock) == 0);

  assert(washere == 1);

  return 0;
}

/* 
 * spin4.c
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

  if (pthread_getprocessors_np(&CPUs) != 0 || CPUs == 1)
    {
      printf("This test is not applicable to this system.\n");
      printf("Either there is only 1 CPU or the no. could not be determined.\n");
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

  assert(pthread_join(t, (void *) &result) == 0);
  assert(result > 1000);

  assert(pthread_spin_destroy(&lock) == 0);

  assert(washere == 1);

  return 0;
}

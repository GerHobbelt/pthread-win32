/* 
 * mutex5.c
 *
 * Confirm the equality/inequality of the various mutex types,
 * and the default not-set value.
 */

#include "test.h"

static pthread_mutexattr_t mxAttr;

int
main()
{
  int mxType = -1;
  int bool = 0;   /* Use to quell GNU compiler warnings. */

  assert(bool = PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_NORMAL);
  assert(bool = PTHREAD_MUTEX_DEFAULT != PTHREAD_MUTEX_ERRORCHECK);
  assert(bool = PTHREAD_MUTEX_DEFAULT != PTHREAD_MUTEX_RECURSIVE);
  assert(bool = PTHREAD_MUTEX_RECURSIVE != PTHREAD_MUTEX_ERRORCHECK);

  assert(bool = PTHREAD_MUTEX_NORMAL == PTHREAD_MUTEX_FAST_NP);
  assert(bool = PTHREAD_MUTEX_RECURSIVE == PTHREAD_MUTEX_RECURSIVE_NP);
  assert(bool = PTHREAD_MUTEX_ERRORCHECK == PTHREAD_MUTEX_ERRORCHECK_NP);

  if (bool == bool)
    {
      assert(pthread_mutexattr_init(&mxAttr) == 0);
      assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
      assert(mxType == PTHREAD_MUTEX_NORMAL);
    }

  return 0;
}

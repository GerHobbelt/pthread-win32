/*
 * Test for pthread_equal.
 *
 * Depends on functions: pthread_create().
 */

#include <pthread.h>
#include <stdio.h>
#include <windows.h>

void * func(void * arg)
{
  Sleep(2000);
  return 0;
}
 
main()
{
        int rc;
        pthread_t t1, t2;
        if (pthread_create(&t1, NULL, func, (void *) 1) != 0)
	  {
	    return 1;
	  }

        if (pthread_create(&t2, NULL, func, (void *) 2) != 0)
	  {
	    return 1;
	  }

        if (pthread_equal(t1, t2))
	  {
	    return 1;
	  }

        if (pthread_equal(t1,t1) == 0)
	  {
	    return 1;
	  }

	/* This is a hack. We don't want to rely on pthread_join
	   yet if we can help it. */
        Sleep(8000);

	/* Success. */
	return 0;
}

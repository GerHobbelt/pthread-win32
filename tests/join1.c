/*
 * Test for pthread_join().
 *
 * Depends on API functions: pthread_create(), pthread_exit().
 */

#include "test.h"

void *
func(void * arg)
{
	Sleep(1000);

	pthread_exit(arg);

	/* Never reached. */
	exit(1);
}

int
main(int argc, char * argv[])
{
	pthread_t id[4];
	int i;
	int result;

	/* Create a few threads and then exit. */
	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_create(&id[i], NULL, func, (void *) i) == 0);
	  }

	for (i = 0; i < 4; i++)
	  {
	    assert(pthread_join(id[i], (void *) &result) == 0);
	    assert(result == i);
	  }

	/* Success. */
	return 0;
}

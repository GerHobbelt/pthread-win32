/*
 * Test for pthread_equal().
 *
 * Depends on API functions: pthread_create().
 */

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

void *
func(void * arg)
{
	for (;;)
	{ /* spin */ }
}

int
main(int argc, char * argv[])
{
	pthread_t id[2];
	int rc;

	/* Create two threads and compare their thread IDs.
	   The threads will chew CPU, but ensure that their
	   IDs will be valid for a long time :-). */

	pthread_create(&id[0], NULL, entry, NULL);
	pthread_create(&id[1], NULL, entry, NULL);

	if (pthread_equal(id[0], id[1]) == 0)
	{
		/* This is impossible. */
		abort();
	}
	
	/* Never reached. */
	return 0;
}

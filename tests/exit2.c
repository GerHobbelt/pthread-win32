/*
 * Test for pthread_exit().
 *
 * Depends on API functions: pthread_create().
 */

#include <pthread.h>
#include <stdio.h>

void *
func(void * arg)
{
	printf("Hello world\n");
	pthread_exit(arg);

	/* Never reached. */
	return 0;
}

int
main(int argc, char * argv[])
{
	pthread_t id[4];
	int i;

	/* Create a few threads, make them say hello and then exit. */

	for (i = 0; i < 4; i++)
	{
		pthread_create(&id[i], NULL, func, (void *) i);
	}

	/* Semantics should be the same as POSIX. Wait for the workers. */
	pthread_exit((void *) 0);
}

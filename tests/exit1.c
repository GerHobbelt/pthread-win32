/*
 * Test for pthread_exit().
 *
 * Depends on API functions: None.
 */

#include <pthread.h>

int
main(int argc, char * argv[])
{
	/* A simple test firstly. */
	pthread_exit((void *) 2);
}

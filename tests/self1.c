/*
 * Test for pthread_self().
 *
 * Depends on API functions: None.
 */

#include <pthread.h>

int
main(int argc, char * argv[])
{
	pthread_t id;

	/* We can't do anything with this, though, because it is not
	   safe to assume anything about the internal structure of
	   a `pthread_t'. */

	id = pthread_self();

	return 0;
}

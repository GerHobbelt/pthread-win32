/*
 * Test for pthread_exit().
 */

#include <pthread.h>

int
main(int argc, char * argv[])
{
  /* Should be the same as return 0; */
  pthread_exit(0);
}

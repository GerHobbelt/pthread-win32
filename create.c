/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include "pthread.h"

int
pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	       void * (*start_routine) (void *), void * arg)
{
  /* Call Win32 CreateThread.
     Map attributes as correctly as possible.
  */
}

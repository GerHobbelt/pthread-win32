/*
 * global.c
 *
 * Description:
 * This translation unit instantiates data associated with the implementation
 * as a whole.
 */

#include "pthread.h"
#include "implement.h"


int _pthread_processInitialized = FALSE;
pthread_key_t _pthread_selfThreadKey = NULL;
pthread_key_t _pthread_cleanupKey = NULL;



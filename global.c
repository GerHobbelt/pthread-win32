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

/*
 * Global lock for testing internal state of PTHREAD_MUTEX_INITIALIZER
 * created mutexes.
 */
CRITICAL_SECTION _pthread_mutex_test_init_lock;

/*
 * Global lock for testing internal state of PTHREAD_COND_INITIALIZER
 * created condition variables.
 */
CRITICAL_SECTION _pthread_cond_test_init_lock;




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

#if !defined(_MSC_VER) && defined(__cplusplus)

Pthread_exception pthread_exception;

#endif

/*
 * Global lock for testing internal state of PTHREAD_MUTEX_INITIALIZER
 * created mutexes.
 */
CRITICAL_SECTION _pthread_mutex_test_init_lock;




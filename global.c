/*
 * global.c
 *
 * Description:
 * This translation unit instantiates data associated with the implementation
 * as a whole.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

pthread_mutex_t _pthread_count_mutex = PTHREAD_MUTEX_INITIALIZER;

DWORD _pthread_threads_count = 0;

/* Per thread management storage. */
_pthread_threads_thread_t _pthread_threads_table[PTHREAD_THREADS_MAX];

/* Per thread mutex locks. */
pthread_mutex_t _pthread_threads_mutex_table[PTHREAD_THREADS_MAX];

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

#define PTHREAD_THREADS_MAX 128
#define PTHREAD_STACK_MIN   65535

/* Convert these to defined when implemented. */
#define _POSIX_THREAD_ATTR_STACKSIZE
#ifdef _POSIX_THREAD_ATTR_STACKADDR
#undef _POSIX_THREAD_ATTR_STACKADDR
#endif

/* Making these constants will mean that applications remain binary
   compatible between versions of the DLL. */

const int _pthread_create_joinable     = 0;
const int _pthread_create_detached     = 1;

/* Cancelability attributes */
const int _pthread_cancel_enable       = 0;
const int _pthread_cancel_disable      = 1;

const int _pthread_cancel_asynchronous = 0;
const int _pthread_cancel_deferred     = 1;


pthread_mutex_t _pthread_table_mutex = PTHREAD_MUTEX_INITIALIZER;

DWORD _pthread_threads_count = 0;

/* Per thread management storage. */
_pthread_threads_thread_t _pthread_threads_table[PTHREAD_THREADS_MAX];

/* Per thread mutex locks. */
pthread_mutex_t _pthread_threads_mutex_table[PTHREAD_THREADS_MAX];

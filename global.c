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

/* Making these constants will mean that applications remain binary
   compatible between versions of the DLL. */

/* POSIX run-time invariant values. (Currently POSIX minimum values) */
const int _POSIX_THREAD_THREADS_MAX = 128;
const int _POSIX_THREAD_DESTRUCTOR_ITERATIONS = 4;
const int _POSIX_THREAD_KEYS_MAX = 128;


const int _pthread_create_joinable     = 0;
const int _pthread_create_detached     = 1;

/* Cancelability attributes */
const int _pthread_cancel_enable       = 0;
const int _pthread_cancel_disable      = 1;

const int _pthread_cancel_asynchronous = 0;
const int _pthread_cancel_deferred     = 1;

/* FIXME: This is temporary. */
#define PTHREAD_MUTEX_INITIALIZER {0}

pthread_mutex_t _pthread_table_mutex = PTHREAD_MUTEX_INITIALIZER;

DWORD _pthread_threads_count = 0;

/* Per thread management storage. See comments in private.c */
_pthread_t * _pthread_virgins;

int _pthread_virgin_next = 0;

pthread_t * _pthread_reuse;

int _pthread_reuse_top = -1;

pthread_t * _pthread_win32handle_map;

/* Per thread mutex locks. */
pthread_mutex_t * _pthread_threads_mutex_table;

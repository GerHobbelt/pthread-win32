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
const int _POSIX_THREAD_THREADS_MAX = _PTHREAD_MAX_THREADS;
const int _POSIX_THREAD_DESTRUCTOR_ITERATIONS = 4;
const int _POSIX_THREAD_KEYS_MAX = 128;


const int _pthread_create_joinable     = 0;
const int _pthread_create_detached     = 1;

/* Cancelability attributes */
const int _pthread_cancel_enable       = 0;
const int _pthread_cancel_disable      = 1;

const int _pthread_cancel_asynchronous = 0;
const int _pthread_cancel_deferred     = 1;


/* Declare variables which are global to all threads in the process. */

pthread_mutex_t _pthread_table_mutex = PTHREAD_MUTEX_INITIALIZER;

DWORD _pthread_threads_count = 0;

/* Per thread management storage. See comments in private.c */
/* An array of struct _pthread */
_pthread_t _pthread_virgins[_PTHREAD_MAX_THREADS];

/* Index to the next available previously unused struct _pthread */
int _pthread_virgin_next = 0;

/* An array of pointers to struct _pthread */
pthread_t _pthread_reuse[_PTHREAD_MAX_THREADS];

/* Index to the first available reusable pthread_t. */
int _pthread_reuse_top = -1;

/* An array of pointers to struct _pthread indexed by hashing
   the Win32 handle. */
pthread_t _pthread_win32handle_map[_PTHREAD_MAX_THREADS];

/* Per thread mutex locks. */
pthread_mutex_t _pthread_threads_mutex_table[_PTHREAD_MAX_THREADS];


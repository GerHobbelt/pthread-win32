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

_pthread_threads_thread_t _pthread_threads_table[PTHREAD_THREADS_MAX];

unsigned short _pthread_once_flag;

pthread_mutex_t _pthread_once_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * implement.h
 *
 * Things that do not belong in pthread.h but may be needed
 * by other parts of this pthreads implementation internally.
 */

/* FIXME: Arbitrary. Need value from Win32.
 */
#define PTHREAD_THREADS_MAX 256;

extern DWORD pthreads_thread_count;

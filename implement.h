/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

/* FIXME: Arbitrary. Need values from Win32.
 */
#define PTHREAD_THREADS_MAX 256
#define PTHREAD_STACK_MIN   65535

extern DWORD pthreads_thread_count;

typedef struct {
  size_t stacksize;
} _pthread_attr_t;

typedef struct {
  int pshared;
} _pthread_mutexattr_t;

typedef struct {
  int pshared;
} _pthread_condattr_t;

#endif /* _IMPLEMENT_H */

/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

/* FIXME: Arbitrary. Need values from Win32.
 */
#define PTHREAD_THREADS_MAX 128
#define PTHREAD_STACK_MIN   65535

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

typedef struct _pthread_cleanup_stack _pthread_cleanup_stack_t;
struct _pthread_cleanup_stck {
  _pthread_cleanup_stack_t first;
  int count;
};

typedef struct _pthread_cleanup_node _pthread_cleanup_node_t;
struct _pthread_cleanup_node {
  _pthread_cleanup_node_t next;
  void (* routine)(void *);
  void * arg;
};

typedef struct {
  size_t stacksize;
} _pthread_attr_t;

typedef struct {
  /* Nothing needed yet. */
} _pthread_mutexattr_t;

typedef struct {
  /* Nothing needed yet. */
} _pthread_condattr_t;

#endif /* _IMPLEMENT_H */



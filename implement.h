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

/* This is all compile time arithmetic */
#define RND_SIZEOF(T) (((sizeof(T) / sizeof(DWORD)) + 1) * sizeof(DWORD))


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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void _pthread_cleanup_push(void (*routine)(void *), void *arg);
void _pthread_cleanup_pop(int execute);
void _pthread_do_cancellation(int tindex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _IMPLEMENT_H */



/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

/* Handler execution flags. */
#define _PTHREAD_HANDLER_NOEXECUTE 0
#define _PTHREAD_HANDLER_EXECUTE   1

/* Handler popping schemes. */
enum { _PTHREAD_HANDLER_POP_LIFO, _PTHREAD_HANDLER_POP_FIFO };

/* Special value to mark attribute objects as valid. */
#define _PTHREAD_ATTR_INVALID 0xC0FFEE

/* Round a sizeof(type) up to a multiple of sizeof(DWORD).
   This is all compile time arithmetic.
 */
#define RND_SIZEOF(T) (((sizeof(T) / sizeof(DWORD)) + 1) * sizeof(DWORD))

/* General description of a handler function on a stack. */
typedef struct _pthread_handler_node _pthread_handler_node_t;

struct _pthread_handler_node {
  _pthread_handler_node_t next;
  void (* routine)(void *);
  void * arg;
};

/* Stores a thread call routine and argument. */
typedef struct {
  unsigned (*routine)(void *);
  void * arg;
  jmpbuf env;
} _pthread_call_t;

#define _PTHREAD_THIS (_pthread_find_thread_entry(pthread_this()))

#define _PTHREAD_STACK(stack) \
  ((_pthread_handler_node_t *) &(_PTHREAD_THIS)->cleanupstack + stack);

/* An element in the thread table. */
typedef struct _pthread_threads_thread _pthread_threads_thread_t;

struct _pthread_threads_thread {
  pthread_t                   thread;
  pthread_attr_t              attr;
  _pthread_call_t             call;
  void **                     joinvalueptr;
  _pthread_handler_node_t *   cleanupstack;
  _pthread_handler_node_t *   destructorstack;
  _pthread_handler_node_t *   forkpreparestack;
  _pthread_handler_node_t *   forkparentstack;
  _pthread_handler_node_t *   forkchildstack;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Generic handler push and pop routines. */

void _pthread_handler_push(_pthread_handler_node_t ** stacktop,
			   int poporder,
			   void (*routine)(void *),
			   void *arg);

void _pthread_handler_pop(_pthread_handler_node_t ** stacktop,
			  int execute);

void _pthread_handler_pop_all(_pthread_handler_node_t ** stacktop,
			      int execute);

/* Primitives to manage threads table entries. */

int _pthread_new_thread_entry(pthread_t thread, 
			      _pthread_threads_thread_t ** entry);

_pthread_threads_thread ** _pthread_find_thread_entry(pthread_t thread);

void _pthread_delete_thread_entry(_pthread_threads_thread_t ** this);

/* Thread cleanup. */

void _pthread_vacuum(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Global data declared in global.c */

extern pthread_mutex_t _pthread_count_mutex;

extern DWORD _pthread_threads_count;

extern _pthread_threads_thread_t _pthread_threads_table[];

extern unsigned short _pthread_once_flag;

extern pthread_mutex_t _pthread_once_lock;


#endif /* _IMPLEMENT_H */


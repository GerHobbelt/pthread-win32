/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

#define _PTHREAD_YES 1
#define _PTHREAD_NO  0

/* Handler execution flags. */
#define _PTHREAD_HANDLER_NOEXECUTE 0
#define _PTHREAD_HANDLER_EXECUTE   1

/* Handler popping schemes. */
enum { _PTHREAD_HANDLER_POP_LIFO, _PTHREAD_HANDLER_POP_FIFO };

/* Special value to mark attribute objects as valid. */
#define _PTHREAD_ATTR_INVALID 0xC0FFEE

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

/* Macro to return the address of the thread entry of the calling thread. */
#define _PTHREAD_THIS (_pthread_find_thread_entry(pthread_this()))

/* Macro to compute the address of a given handler stack. */
#define _PTHREAD_STACK(stack) \
  ((_pthread_handler_node_t *) &(_PTHREAD_THIS)->cleanupstack + stack);

/* Macro to compute the table index of a thread entry from it's entry
   address. */
#define _PTHREAD_THREADS_TABLE_INDEX(this) \
  ((_pthread_threads_table_t *) this - \
   (_pthread_threads_table_t *) _pthread_threads_threads_table)

/* Macro to compute the address of a per-thread mutex lock. */
#define _PTHREAD_THREAD_MUTEX(this) \
   (&_pthread_threads_mutex_table[_PTHREAD_THREADS_TABLE_INDEX(this)])

/* An element in the thread table. */
typedef struct _pthread_threads_thread _pthread_threads_thread_t;

struct _pthread_threads_thread {
  pthread_t                   thread;
  pthread_attr_t              attr;
  _pthread_call_t             call;
  int                         cancel_pending;
  int                         cancelstate;      /* PTHREAD_CANCEL_DISABLE
						   PTHREAD_CANCEL_ENABLE */

  int                         canceltype;       /* PTHREAD_CANCEL_ASYNCHRONOUS
						   PTHREAD_CANCEL_DEFERRED */
  void **                     joinvalueptr;
  int                         join_count;
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

int _pthread_handler_push(int stack,
			  int poporder,
			  void (*routine)(void *),
			  void *arg);

void _pthread_handler_pop(int stack,
			  int execute);

void _pthread_handler_pop_all(int stack,
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

extern pthread_mutex_t _pthread_threads_mutex_table[];

#endif /* _IMPLEMENT_H */


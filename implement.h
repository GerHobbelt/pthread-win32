/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

enum {
  _PTHREAD_NEW,
  _PTHREAD_INUSE,
  _PTHREAD_EXITED,
  _PTHREAD_REUSE
};

#define _PTHREAD_VALID(T) \
  ((T) != NULL \
   && ((T)->ptstatus == _PTHREAD_NEW \
       || (T)->ptstatus == _PTHREAD_INUSE))

/* Handler execution flags. */
#define _PTHREAD_HANDLER_NOEXECUTE 0
#define _PTHREAD_HANDLER_EXECUTE   1

/* Special value to mark attribute objects as valid. */
#define _PTHREAD_ATTR_VALID 0xC0FFEE

/* General description of a handler function on a stack. */
typedef struct _pthread_handler_node _pthread_handler_node_t;

struct _pthread_handler_node {
  _pthread_handler_node_t * next;
  void (* routine)(void *);
  void * arg;
};

/* Stores a thread call routine and argument. */
typedef struct {
  unsigned (*routine)(void *);
  void * arg;
} _pthread_call_t;

/* Macro to compute the address of a given handler stack. */
#define _PTHREAD_STACK(stack) \
  ((_pthread_handler_node_t *) &(pthread_self())->cleanupstack + stack);

/* Macro to compute the table index of a thread entry from it's entry
   address. */
#define _PTHREAD_THREADS_TABLE_INDEX(this) \
  ((_pthread_threads_table_t *) this - \
   (_pthread_threads_table_t *) _pthread_threads_threads_table)

/* Macro to compute the address of a per-thread mutex lock. */
#define _PTHREAD_THREAD_MUTEX(this) \
   (&_pthread_threads_mutex_table[_PTHREAD_THREADS_TABLE_INDEX(this)])

/* An element in the thread table. */
typedef struct _pthread _pthread_t;

/* Keep the old typedef until we've updated all source files. */
typedef struct _pthread _pthread_threads_thread_t;

/*                                                 Related constants */
struct _pthread {
  HANDLE                      win32handle;
  int                         ptstatus;        /* _PTHREAD_EXITED
						   _PTHREAD_REUSABLE */
  pthread_attr_t              attr;
  _pthread_call_t             call;
  int                         cancel_pending;
  int                         cancelstate;      /* PTHREAD_CANCEL_DISABLE
						   PTHREAD_CANCEL_ENABLE */

  int                         canceltype;       /* PTHREAD_CANCEL_ASYNCHRONOUS
						   PTHREAD_CANCEL_DEFERRED */
  void **                     joinvalueptr;
  int                         join_count;

  /* These must be kept in this order and together. */
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

int _pthread_destructor_push(void (*routine)(void *),
			     pthread_key_t key);

void _pthread_destructor_pop(pthread_key_t key);

void _pthread_destructor_pop_all();

/* Primitives to manage threads table entries. */

int _pthread_new_thread(pthread_t * thread);

int _pthread_delete_thread(pthread_t thread);

/* Thread cleanup. */

void _pthread_vacuum(void);

void _pthread_exit(pthread_t thread, void * value, int return_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Global declared dll.c */

extern DWORD _pthread_threadID_TlsIndex;


/* Global data declared in global.c */

extern pthread_mutex_t _pthread_table_mutex;

extern DWORD _pthread_threads_count;

/* An array of struct _pthread */
extern _pthread_t _pthread_virgins[];

/* Index to the next available previously unused struct _pthread */
extern int _pthread_virgin_next;

/* An array of pointers to struct _pthread */
extern pthread_t _pthread_reuse[];

/* Index to the first available reusable pthread_t. */
extern int _pthread_reuse_top;

/* An array of pointers to struct _pthread indexed by hashing
   the Win32 handle. */
extern pthread_t _pthread_win32handle_map[];

/* Per thread mutex locks. */
extern pthread_mutex_t _pthread_threads_mutex_table[];

#endif /* _IMPLEMENT_H */





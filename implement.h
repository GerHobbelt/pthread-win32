/*
 * implement.h
 *
 * Definitions that don't need to be public.
 *
 * Keeps all the internals out of pthread.h
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

/*
 * Code contributed by John E. Bossom <JEB>.
 */

typedef enum {
  /*
   * This enumeration represents the state of the thread;
   * The thread is still "alive" if the numeric value of the
   * state is greater or equal "PThreadStateRunning".
   */
  PThreadStateInitial = 0,	/* Thread not running                   */
  PThreadStateRunning,	        /* Thread alive & kicking               */
  PThreadStateSuspended,	/* Thread alive but suspended           */
  PThreadStateCanceling,	/* Thread alive but and is              */
                                /* in the process of terminating        */
                                /* due to a cancellation request        */
  PThreadStateException,	/* Thread alive but exiting             */
                                /* due to an exception                  */
  PThreadStateLast
}
PThreadState;


typedef enum {
  /*
   * This enumeration represents the reason why a thread has
   * terminated/is terminating.
   */
  PThreadDemisePeaceful = 0,	/* Death due natural causes     */
  PThreadDemiseCancelled,	/* Death due to user cancel     */
  PThreadDemiseException,	/* Death due to unhandled       */
                                /* exception                    */
  PThreadDemiseNotDead	/* I'm not dead!                */
}
PThreadDemise;


struct pthread_t_ {
  DWORD thread;
  HANDLE threadH;
  PThreadState state;
  PThreadDemise demise;
  void *exitStatus;
  void *parms;
  int detachState;
  int cancelState;
  int cancelType;
  HANDLE cancelEvent;
  int implicit:1;
  void *keys;
};


struct pthread_attr_t_ {
  void *stackaddr;
  size_t stacksize;
  int detachstate;
};


struct pthread_key_t_ {
  DWORD key;
  void (*destructor) (void *);
  pthread_mutex_t threadsLock;
  void *threads;
};


struct pthread_mutexattr_t_ {
  int pshared;
};


struct pthread_mutex_t_ {
	int valid;
	CRITICAL_SECTION cs;
  };


struct pthread_cond_t_ {
  long waiters;                       /* # waiting threads             */
  pthread_mutex_t waitersLock;        /* Mutex that guards access to 
					 waiter count                  */
  sem_t sema;                         /* Queue up threads waiting for the 
					 condition to become signaled  */
  HANDLE waitersDone;                 /* An auto reset event used by the 
					 broadcast/signal thread to wait 
					 for the waiting thread(s) to wake
					 up and get a chance at the  
					 semaphore                     */
  int wasBroadcast;                   /* keeps track if we are signaling 
					 or broadcasting               */
};


struct pthread_condattr_t_ {
  int pshared;
};


struct pthread_once_t_ {
  unsigned short flag;
  pthread_mutex_t lock;
};


typedef struct ThreadParms ThreadParms;
typedef struct ThreadKeyAssoc ThreadKeyAssoc;


struct ThreadParms {
  pthread_t tid;
  void *(*start) (void *);
  void *arg;
};


struct ThreadKeyAssoc {
  /*
   * Purpose:
   *      This structure creates an association between a
   *      thread and a key.
   *      It is used to implement the implicit invocation
   *      of a user defined destroy routine for thread
   *      specific data registered by a user upon exiting a
   *      thread.
   *
   * Attributes:
   *      lock
   *              protects access to the rest of the structure
   *
   *      thread
   *              reference to the thread that owns the association.
   *              As long as this is not NULL, the association remains
   *              referenced by the pthread_t.
   *
   *      key
   *              reference to the key that owns the association.
   *              As long as this is not NULL, the association remains
   *              referenced by the pthread_key_t.
   *
   *      nextKey
   *              The pthread_t->keys attribute is the head of a
   *              chain of associations that runs through the nextKey
   *              link. This chain provides the 1 to many relationship
   *              between a pthread_t and all pthread_key_t on which
   *              it called pthread_setspecific.
   *
   *      nextThread
   *              The pthread_key_t->threads attribute is the head of
   *              a chain of assoctiations that runs through the
   *              nextThreads link. This chain provides the 1 to many
   *              relationship between a pthread_key_t and all the 
   *              PThreads that have called pthread_setspecific for
   *              this pthread_key_t.
   *
   *
   * Notes:
   *      1)      As long as one of the attributes, thread or key, is
   *              not NULL, the association is being referenced; once
   *              both are NULL, the association must be released.
   *
   *      2)      Under WIN32, an association is only created by
   *              pthread_setspecific if the user provided a
   *              destroyRoutine when they created the key.
   *
   *
   */
  pthread_mutex_t lock;
  pthread_t thread;
  pthread_key_t key;
  ThreadKeyAssoc *nextKey;
  ThreadKeyAssoc *nextThread;
};


/*
 * --------------------------------------------------------------
 * MAKE_SOFTWARE_EXCEPTION
 *      This macro constructs a software exception code following
 *      the same format as the standard Win32 error codes as defined
 *      in WINERROR.H
 *  Values are 32 bit values layed out as follows:
 *
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *  +---+-+-+-----------------------+-------------------------------+
 *  |Sev|C|R|     Facility          |               Code            |
 *  +---+-+-+-----------------------+-------------------------------+
 *
 * Severity Values:
 */
#define SE_SUCCESS              0x00
#define SE_INFORMATION	        0x01
#define SE_WARNING              0x10
#define SE_ERROR                0x11

#define MAKE_SOFTWARE_EXCEPTION( _severity, _facility, _exception ) \
( (DWORD) ( ( (_severity) << 30 ) |	/* Severity code	*/ \
	    ( 1 << 29 )	|		/* MS=0, User=1		*/ \
	    ( 0 << 28 )	|		/* Reserved		*/ \
	    ( (_facility) << 16 ) |	/* Facility Code	*/ \
	    ( (_exception) <<  0 )	/* Exception Code	*/ \
	    ) )

/*
 * We choose one specific Facility/Error code combination to
 * identify our software exceptions vs. WIN32 exceptions.
 * We store our actual component and error code within
 * the optional information array.
 */
#define EXCEPTION_PTHREAD_SERVICES	\
     MAKE_SOFTWARE_EXCEPTION( SE_ERROR, \
			      PTHREAD_SERVICES_FACILITY, \
			      PTHREAD_SERVICES_ERROR )


#define PTHREAD_SERVICES_FACILITY		0xBAD
#define PTHREAD_SERVICES_ERROR			0xDEED


/* Function pointer to TryEnterCriticalSection if it exists; otherwise NULL */
extern BOOL (WINAPI *_pthread_try_enter_critical_section)(LPCRITICAL_SECTION);

/* Declared in global.c */
extern int _pthread_processInitialized;
extern pthread_key_t _pthread_selfThreadKey;
extern pthread_key_t _pthread_cleanupKey;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * =====================
 * =====================
 * Forward Declarations
 * =====================
 * =====================
 */
int _pthread_processInitialize (void);

void _pthread_processTerminate (void);

void _pthread_threadDestroy (pthread_t tid);

void _pthread_cleanupStack (void);

void *_pthread_threadStart (ThreadParms * threadParms);

void _pthread_callUserDestroyRoutines (pthread_t thread);

int _pthread_tkAssocCreate (ThreadKeyAssoc ** assocP,
			    pthread_t thread,
			    pthread_key_t key);

void _pthread_tkAssocDestroy (ThreadKeyAssoc * assoc);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* </JEB> */


#if 0 /* Pre Bossom */

/* Use internally to initialise const ints and thread admin array sizes. */
#define _PTHREAD_MAX_THREADS 128
#define _PTHREAD_MAX_KEYS 128

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

enum {
  _PTHREAD_NEW,
  _PTHREAD_INUSE,
  _PTHREAD_EXITED,
  _PTHREAD_REUSE
};

enum {
  _PTHREAD_TSD_KEY_DELETED,
  _PTHREAD_TSD_KEY_INUSE,
  _PTHREAD_TSD_KEY_REUSE
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

/* TSD key element. */
typedef struct _pthread_tsd_key _pthread_tsd_key_t;

struct _pthread_tsd_key {
  int in_use;
  int status;
  void (* destructor)(void *);
};

/* Stores a thread call routine and argument. */
typedef struct {
  unsigned (*routine)(void *);
  void * arg;
} _pthread_call_t;

/* Macro to compute the address of a given handler stack. */
#define _PTHREAD_STACK(stack) \
  ((_pthread_handler_node_t **) &(pthread_self()->cleanupstack) + stack);

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

void _pthread_destructor_run_all();

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

extern DWORD _pthread_TSD_keys_TlsIndex;


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

/* Global TSD key array. */
extern _pthread_tsd_key_t _pthread_tsd_key_table[];

/* Mutex lock for TSD operations */
extern pthread_mutex_t _pthread_tsd_mutex;

/* Function pointer to TryEnterCriticalSection if it exists; otherwise NULL */
extern BOOL (WINAPI *_pthread_try_enter_critical_section)(LPCRITICAL_SECTION);

/* An array of pthread_key_t */
extern pthread_key_t _pthread_key_virgins[];

/* Index to the next available previously unused pthread_key_t */
extern int _pthread_key_virgin_next;

/* An array of pthread_key_t */
extern pthread_key_t _pthread_key_reuse[];

/* Index to the first available reusable pthread_key_t. */
extern int _pthread_key_reuse_top;

#endif /* Pre Bossom */

#endif /* _IMPLEMENT_H */

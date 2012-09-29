/*
 * implement.h
 *
 * Definitions that don't need to be public.
 *
 * Keeps all the internals out of pthread.h
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x400

#include <windows.h>

/*
 * In case windows.h doesn't define it (e.g. WinCE perhaps)
 */
#ifdef WINCE
typedef VOID (APIENTRY *PAPCFUNC)(DWORD dwParam);
#endif

/*
 * note: ETIMEDOUT is correctly defined in winsock.h
 */
#include <winsock.h>

/*
 * In case ETIMEDOUT hasn't been defined above somehow.
 */
#ifndef ETIMEDOUT
#  define ETIMEDOUT 10060	/* This is the value in winsock.h. */
#endif

#if !defined(malloc)
#include <malloc.h>
#endif

#if !defined(INT_MAX)
#include <limits.h>
#endif

/* use local include files during development */
#include "semaphore.h"
#include "sched.h"

#if defined(HAVE_C_INLINE) || defined(__cplusplus)
#define INLINE inline
#else
#define INLINE
#endif

#if defined (__MINGW32__) || (_MSC_VER >= 1300)
#define PTE_INTERLOCKED_LONG long
#define PTE_INTERLOCKED_LPLONG long*
#else
#define PTE_INTERLOCKED_LONG PVOID
#define PTE_INTERLOCKED_LPLONG PVOID*
#endif

#if defined(__MINGW32__)
#include <stdint.h>
#elif defined(__BORLANDC__)
#define int64_t ULONGLONG
#else
#define int64_t _int64
#endif

typedef enum
{
  /*
   * This enumeration represents the state of the thread;
   * The thread is still "alive" if the numeric value of the
   * state is greater or equal "PThreadStateRunning".
   */
  PThreadStateInitial = 0,	/* Thread not running                   */
  PThreadStateRunning,		/* Thread alive & kicking               */
  PThreadStateSuspended,	/* Thread alive but suspended           */
  PThreadStateCancelPending,	/* Thread alive but is                  */
  /* has cancelation pending.        */
  PThreadStateCanceling,	/* Thread alive but is                  */
  /* in the process of terminating        */
  /* due to a cancellation request        */
  PThreadStateException,	/* Thread alive but exiting             */
  /* due to an exception                  */
  PThreadStateLast
}
PThreadState;


typedef struct pte_thread_t_ pte_thread_t;

struct pte_thread_t_
{
#ifdef _UWIN
  DWORD dummy[5];
#endif
  DWORD thread;
  HANDLE threadH;		/* Win32 thread handle - POSIX thread is invalid if threadH == 0 */
  pthread_t ptHandle;		/* This thread's permanent pthread_t handle */
  pte_thread_t * prevReuse;	/* Links threads on reuse stack */
  volatile PThreadState state;
  void *exitStatus;
  void *parms;
  int ptErrno;
  int detachState;
  pthread_mutex_t threadLock;	/* Used for serialised access to public thread state */
  int sched_priority;		/* As set, not as currently is */
  pthread_mutex_t cancelLock;	/* Used for async-cancel safety */
  int cancelState;
  int cancelType;
  HANDLE cancelEvent;
#ifdef __CLEANUP_C
  jmp_buf start_mark;
#endif				/* __CLEANUP_C */
#if HAVE_SIGSET_T
  sigset_t sigmask;
#endif				/* HAVE_SIGSET_T */
  int implicit:1;
  void *keys;
  void *nextAssoc;
};


/* 
 * Special value to mark attribute objects as valid.
 */
#define PTE_ATTR_VALID ((unsigned long) 0xC4C0FFEE)

struct pthread_attr_t_
{
  unsigned long valid;
  void *stackaddr;
  size_t stacksize;
  int detachstate;
  struct sched_param param;
  int inheritsched;
  int contentionscope;
#if HAVE_SIGSET_T
  sigset_t sigmask;
#endif				/* HAVE_SIGSET_T */
};


/*
 * ====================
 * ====================
 * Semaphores, Mutexes and Condition Variables
 * ====================
 * ====================
 */

struct sem_t_
{
  int value;
  pthread_mutex_t lock;
  HANDLE sem;
#ifdef NEED_SEM
  int leftToUnblock;
#endif
};

#define PTE_OBJECT_AUTO_INIT ((void *) -1)
#define PTE_OBJECT_INVALID   NULL

struct pthread_mutex_t_
{
  LONG lock_idx;		/* Provides exclusive access to mutex state
				   via the Interlocked* mechanism.
				    0: unlocked/free.
				    1: locked - no other waiters.
				   -1: locked - with possible other waiters.
				*/
  int recursive_count;		/* Number of unlocks a thread needs to perform
				   before the lock is released (recursive
				   mutexes only). */
  int kind;			/* Mutex type. */
  pthread_t ownerThread;
  HANDLE event;			/* Mutex release notification to waiting
				   threads. */
};

struct pthread_mutexattr_t_
{
  int pshared;
  int kind;
};

/*
 * Possible values, other than PTE_OBJECT_INVALID,
 * for the "interlock" element in a spinlock.
 *
 * In this implementation, when a spinlock is initialised,
 * the number of cpus available to the process is checked.
 * If there is only one cpu then "interlock" is set equal to
 * PTE_SPIN_USE_MUTEX and u.mutex is a initialised mutex.
 * If the number of cpus is greater than 1 then "interlock"
 * is set equal to PTE_SPIN_UNLOCKED and the number is
 * stored in u.cpus. This arrangement allows the spinlock
 * routines to attempt an InterlockedCompareExchange on "interlock"
 * immediately and, if that fails, to try the inferior mutex.
 *
 * "u.cpus" isn't used for anything yet, but could be used at
 * some point to optimise spinlock behaviour.
 */
#define PTE_SPIN_UNLOCKED    (1)
#define PTE_SPIN_LOCKED      (2)
#define PTE_SPIN_USE_MUTEX   (3)

struct pthread_spinlock_t_
{
  long interlock;		/* Locking element for multi-cpus. */
  union
  {
    int cpus;			/* No. of cpus if multi cpus, or   */
    pthread_mutex_t mutex;	/* mutex if single cpu.            */
  } u;
};

struct pthread_barrier_t_
{
  unsigned int nCurrentBarrierHeight;
  unsigned int nInitialBarrierHeight;
  int iStep;
  int pshared;
  sem_t semBarrierBreeched[2];
};

struct pthread_barrierattr_t_
{
  int pshared;
};

struct pthread_key_t_
{
  DWORD key;
  void (*destructor) (void *);
  pthread_mutex_t keyLock;
  void *threads;
};


typedef struct ThreadParms ThreadParms;
typedef struct ThreadKeyAssoc ThreadKeyAssoc;

struct ThreadParms
{
  pthread_t tid;
  void *(*start) (void *);
  void *arg;
};


struct pthread_cond_t_
{
  long nWaitersBlocked;		/* Number of threads blocked            */
  long nWaitersGone;		/* Number of threads timed out          */
  long nWaitersToUnblock;	/* Number of threads to unblock         */
  sem_t semBlockQueue;		/* Queue up threads waiting for the     */
  /*   condition to become signalled      */
  sem_t semBlockLock;		/* Semaphore that guards access to      */
  /* | waiters blocked count/block queue  */
  /* +-> Mandatory Sync.LEVEL-1           */
  pthread_mutex_t mtxUnblockLock;	/* Mutex that guards access to          */
  /* | waiters (to)unblock(ed) counts     */
  /* +-> Optional* Sync.LEVEL-2           */
  pthread_cond_t next;		/* Doubly linked list                   */
  pthread_cond_t prev;
};


struct pthread_condattr_t_
{
  int pshared;
};

#define PTE_RWLOCK_MAGIC 0xfacade2

struct pthread_rwlock_t_
{
  pthread_mutex_t mtxExclusiveAccess;
  pthread_mutex_t mtxSharedAccessCompleted;
  pthread_cond_t cndSharedAccessCompleted;
  int nSharedAccessCount;
  int nExclusiveAccessCount;
  int nCompletedSharedAccessCount;
  int nMagic;
};

struct pthread_rwlockattr_t_
{
  int pshared;
};

/*
 * MCS lock queue node - see pte_MCS_lock.c
 */
struct pte_mcs_node_t_
{
  struct pte_mcs_node_t_ **lock;        /* ptr to tail of queue */
  struct pte_mcs_node_t_  *next;        /* ptr to successor in queue */
  LONG                       readyFlag;   /* set after lock is released by
                                             predecessor */
  LONG                       nextFlag;    /* set after 'next' ptr is set by
                                             successor */
};

typedef struct pte_mcs_node_t_   pte_mcs_local_node_t;
typedef struct pte_mcs_node_t_  *pte_mcs_lock_t;


struct ThreadKeyAssoc
{
  /*
   * Purpose:
   *      This structure creates an association between a thread and a key.
   *      It is used to implement the implicit invocation of a user defined
   *      destroy routine for thread specific data registered by a user upon
   *      exiting a thread.
   *
   *      Graphically, the arrangement is as follows, where:
   *
   *         K - Key with destructor
   *            (head of chain is key->threads)
   *         T - Thread that has called pthread_setspecific(Kn)
   *            (head of chain is thread->keys)
   *         A - Association. Each association is a node at the
   *             intersection of two doubly-linked lists.
   *
   *                 T1    T2    T3
   *                 |     |     |
   *                 |     |     |
   *         K1 -----+-----A-----A----->
   *                 |     |     |
   *                 |     |     |
   *         K2 -----A-----A-----+----->
   *                 |     |     |
   *                 |     |     |
   *         K3 -----A-----+-----A----->
   *                 |     |     |
   *                 |     |     |
   *                 V     V     V
   *
   *      Access to the association is guarded by two locks: the key's
   *      general lock (guarding the row) and the thread's general
   *      lock (guarding the column). This avoids the need for a
   *      dedicated lock for each association, which not only consumes
   *      more handles but requires that: before the lock handle can
   *      be released - both the key must be deleted and the thread
   *      must have called the destructor. The two-lock arrangement
   *      allows the resources to be freed as soon as either thread or
   *      key is concluded.
   *
   *      To avoid deadlock: whenever both locks are required, the key
   *      and thread locks are always acquired in the order: key lock
   *      then thread lock. An exception to this exists when a thread
   *      calls the destructors, however this is done carefully to
   *      avoid deadlock.
   *
   *      An association is created when a thread first calls
   *      pthread_setspecific() on a key that has a specified
   *      destructor.
   *
   *      An association is destroyed either immediately after the
   *      thread calls the key destructor function on thread exit, or
   *      when the key is deleted.
   *
   * Attributes:
   *      thread
   *              reference to the thread that owns the
   *              association. This is actually the pointer to the
   *              thread struct itself. Since the association is
   *              destroyed before the thread exits, this can never
   *              point to a different logical thread to the one that
   *              created the assoc, i.e. after thread struct reuse.
   *
   *      key
   *              reference to the key that owns the association.
   *
   *      nextKey
   *              The pthread_t->keys attribute is the head of a
   *              chain of associations that runs through the nextKey
   *              link. This chain provides the 1 to many relationship
   *              between a pthread_t and all pthread_key_t on which
   *              it called pthread_setspecific.
   *
   *      prevKey
   *              Similarly.
   *
   *      nextThread
   *              The pthread_key_t->threads attribute is the head of
   *              a chain of assoctiations that runs through the
   *              nextThreads link. This chain provides the 1 to many
   *              relationship between a pthread_key_t and all the 
   *              PThreads that have called pthread_setspecific for
   *              this pthread_key_t.
   *
   *      prevThread
   *              Similarly.
   *
   * Notes:
   *      1)      As soon as either the key or the thread is no longer
   *              referencing the association, it can be destroyed. The
   *              association will be removed from both chains.
   *
   *      2)      Under WIN32, an association is only created by
   *              pthread_setspecific if the user provided a
   *              destroyRoutine when they created the key.
   *
   *
   */
  pte_thread_t * thread;
  pthread_key_t key;
  ThreadKeyAssoc *nextKey;
  ThreadKeyAssoc *nextThread;
  ThreadKeyAssoc *prevKey;
  ThreadKeyAssoc *prevThread;
};


#ifdef __CLEANUP_SEH
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
#define SE_INFORMATION          0x01
#define SE_WARNING              0x02
#define SE_ERROR                0x03

#define MAKE_SOFTWARE_EXCEPTION( _severity, _facility, _exception ) \
( (DWORD) ( ( (_severity) << 30 ) |     /* Severity code        */ \
            ( 1 << 29 ) |               /* MS=0, User=1         */ \
            ( 0 << 28 ) |               /* Reserved             */ \
            ( (_facility) << 16 ) |     /* Facility Code        */ \
            ( (_exception) <<  0 )      /* Exception Code       */ \
            ) )

/*
 * We choose one specific Facility/Error code combination to
 * identify our software exceptions vs. WIN32 exceptions.
 * We store our actual component and error code within
 * the optional information array.
 */
#define EXCEPTION_PTE_SERVICES        \
     MAKE_SOFTWARE_EXCEPTION( SE_ERROR, \
                              PTE_SERVICES_FACILITY, \
                              PTE_SERVICES_ERROR )

#define PTE_SERVICES_FACILITY         0xBAD
#define PTE_SERVICES_ERROR            0xDEED

#endif /* __CLEANUP_SEH */

/*
 * Services available through EXCEPTION_PTE_SERVICES
 * and also used [as parameters to pte_throw()] as
 * generic exception selectors.
 */

#define PTE_EPS_EXIT                  (1)
#define PTE_EPS_CANCEL                (2)


/* Useful macros */
#define PTE_MAX(a,b)  ((a)<(b)?(b):(a))
#define PTE_MIN(a,b)  ((a)>(b)?(b):(a))


/* Declared in global.c */
extern PTE_INTERLOCKED_LONG (WINAPI *
			       pte_interlocked_compare_exchange)
  (PTE_INTERLOCKED_LPLONG, PTE_INTERLOCKED_LONG, PTE_INTERLOCKED_LONG);

/* Declared in pthread_cancel.c */
extern DWORD (*pte_register_cancelation) (PAPCFUNC, HANDLE, DWORD);

/* Thread Reuse stack bottom marker. Must not be NULL or any valid pointer to memory. */
#define PTE_THREAD_REUSE_EMPTY ((pte_thread_t *) 1)

extern int pte_processInitialized;
extern pte_thread_t * pte_threadReuseTop;
extern pte_thread_t * pte_threadReuseBottom;
extern pthread_key_t pte_selfThreadKey;
extern pthread_key_t pte_cleanupKey;
extern pthread_cond_t pte_cond_list_head;
extern pthread_cond_t pte_cond_list_tail;

extern int pte_mutex_default_kind;

extern int pte_concurrency;

extern int pte_features;

extern BOOL pte_smp_system;  /* True: SMP system, False: Uni-processor system */

extern CRITICAL_SECTION pte_thread_reuse_lock;
extern CRITICAL_SECTION pte_mutex_test_init_lock;
extern CRITICAL_SECTION pte_cond_list_lock;
extern CRITICAL_SECTION pte_cond_test_init_lock;
extern CRITICAL_SECTION pte_rwlock_test_init_lock;
extern CRITICAL_SECTION pte_spinlock_test_init_lock;

#ifdef _UWIN
extern int pthread_count;
#endif

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/*
 * =====================
 * =====================
 * Forward Declarations
 * =====================
 * =====================
 */

  int pte_is_attr (const pthread_attr_t * attr);

  int pte_cond_check_need_init (pthread_cond_t * cond);
  int pte_mutex_check_need_init (pthread_mutex_t * mutex);
  int pte_rwlock_check_need_init (pthread_rwlock_t * rwlock);

  PTE_INTERLOCKED_LONG WINAPI
    pte_InterlockedCompareExchange (PTE_INTERLOCKED_LPLONG location,
				      PTE_INTERLOCKED_LONG value,
				      PTE_INTERLOCKED_LONG comparand);

  LONG WINAPI
    pte_InterlockedExchange (LPLONG location,
			       LONG value);

  DWORD
    pte_RegisterCancelation (PAPCFUNC callback,
			       HANDLE threadH, DWORD callback_arg);

  int pte_processInitialize (void);

  void pte_processTerminate (void);

  void pte_threadDestroy (pthread_t tid);

  void pte_pop_cleanup_all (int execute);

  pthread_t pte_new (void);

  pthread_t pte_threadReusePop (void);

  void pte_threadReusePush (pthread_t thread);

  int pte_getprocessors (int *count);

  int pte_setthreadpriority (pthread_t thread, int policy, int priority);

  void pte_rwlock_cancelwrwait (void *arg);

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
  unsigned __stdcall
#else
  void
#endif
    pte_threadStart (void *vthreadParms);

  void pte_callUserDestroyRoutines (pthread_t thread);

  int pte_tkAssocCreate (pte_thread_t * thread, pthread_key_t key);

  void pte_tkAssocDestroy (ThreadKeyAssoc * assoc);

  int pte_semwait (sem_t * sem);

  DWORD pte_relmillisecs (const struct timespec * abstime);

  void pte_mcs_lock_acquire (pte_mcs_lock_t * lock, pte_mcs_local_node_t * node);

  void pte_mcs_lock_release (pte_mcs_local_node_t * node);

#ifdef NEED_FTIME
  void pte_timespec_to_filetime (const struct timespec *ts, FILETIME * ft);
  void pte_filetime_to_timespec (const FILETIME * ft, struct timespec *ts);
#endif

/* Declared in misc.c */
#ifdef NEED_CALLOC
#define calloc(n, s) pte_calloc(n, s)
  void *pte_calloc (size_t n, size_t s);
#endif

/* Declared in private.c */
  void pte_throw (DWORD exception);

#ifdef __cplusplus
}
#endif				/* __cplusplus */


#ifdef _UWIN_
#   ifdef       _MT
#       ifdef __cplusplus
extern "C"
{
#       endif
  _CRTIMP unsigned long __cdecl _beginthread (void (__cdecl *) (void *),
					      unsigned, void *);
  _CRTIMP void __cdecl _endthread (void);
  _CRTIMP unsigned long __cdecl _beginthreadex (void *, unsigned,
						unsigned (__stdcall *) (void *),
						void *, unsigned, unsigned *);
  _CRTIMP void __cdecl _endthreadex (unsigned);
#       ifdef __cplusplus
}
#       endif
#   endif
#else
#   include <process.h>
#endif


/*
 * Defaults. Could be overridden when building the inlined version of the dll.
 * See pte_InterlockedCompareExchange.c
 */
#ifndef PTE_INTERLOCKED_COMPARE_EXCHANGE
#define PTE_INTERLOCKED_COMPARE_EXCHANGE pte_interlocked_compare_exchange
#endif

#ifndef PTE_INTERLOCKED_EXCHANGE
#define PTE_INTERLOCKED_EXCHANGE InterlockedExchange
#endif


/*
 * Check for old and new versions of cygwin. See the FAQ file:
 *
 * Question 1 - How do I get pthreads-win32 to link under Cygwin or Mingw32?
 *
 * Patch by Anders Norlander <anorland@hem2.passagen.se>
 */
#if defined(__CYGWIN32__) || defined(__CYGWIN__) || defined(NEED_CREATETHREAD)

/* 
 * Macro uses args so we can cast start_proc to LPTHREAD_START_ROUTINE
 * in order to avoid warnings because of return type
 */

#define _beginthreadex(security, \
                       stack_size, \
                       start_proc, \
                       arg, \
                       flags, \
                       pid) \
        CreateThread(security, \
                     stack_size, \
                     (LPTHREAD_START_ROUTINE) start_proc, \
                     arg, \
                     flags, \
                     pid)

#define _endthreadex ExitThread

#endif				/* __CYGWIN32__ || __CYGWIN__ || NEED_CREATETHREAD */


#endif				/* _IMPLEMENT_H */

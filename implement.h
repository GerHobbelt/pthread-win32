/*
 * implement.h
 *
 * Definitions that don't need to be public.
 *
 * Keeps all the internals out of pthread.h
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#ifdef __MINGW32__
#define PT_STDCALL
#else
#ifdef __cplusplus
#define PT_STDCALL __stdcall
#else
#define PT_STDCALL __stdcall
#endif
#endif

/* changed include from <semaphore.h> to use local file during development */
#include "semaphore.h"

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
  int ptErrno;
  int detachState;
  pthread_mutex_t cancelLock;  /* Used for async-cancel safety */
  int cancelState;
  int cancelType;
  HANDLE cancelEvent;
#if HAVE_SIGSET_T
  sigset_t sigmask;
#endif /* HAVE_SIGSET_T */
  int implicit:1;
  void *keys;
};


/* 
 * Special value to mark attribute objects as valid.
 */
#define PTW32_ATTR_VALID ((unsigned long) 0xC4C0FFEE)

struct pthread_attr_t_ {
  unsigned long valid;
  void *stackaddr;
  size_t stacksize;
  int detachstate;
  int priority;
#if HAVE_SIGSET_T
  sigset_t sigmask;
#endif /* HAVE_SIGSET_T */
};


/*
 * ====================
 * ====================
 * Mutexes and Condition Variables
 * ====================
 * ====================
 */

#define PTW32_OBJECT_AUTO_INIT ((void *) -1)
#define PTW32_OBJECT_INVALID   NULL

struct pthread_mutex_t_ {
  HANDLE mutex;
  CRITICAL_SECTION cs;
};


struct pthread_mutexattr_t_ {
  int pshared;
  int forcecs;
};


struct pthread_key_t_ {
  DWORD key;
  void (*destructor) (void *);
  pthread_mutex_t threadsLock;
  void *threads;
};


typedef struct ThreadParms ThreadParms;
typedef struct ThreadKeyAssoc ThreadKeyAssoc;

struct ThreadParms {
  pthread_t tid;
  void *(*start) (void *);
  void *arg;
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

#define RW_MAGIC    0x19283746

struct pthread_rwlock_t_ {
    pthread_mutex_t rw_lock;         /* basic lock on this struct */
    pthread_cond_t  rw_condreaders;  /* for reader threads waiting */
    pthread_cond_t  rw_condwriters;  /* for writer threads waiting */
    int             rw_magic;        /* for error checking */
    int             rw_nwaitreaders; /* the number waiting */
    int             rw_nwaitwriters; /* the number waiting */
    int             rw_refcount;     /* -1 if writer has the lock,
                                        else # readers holding the lock */
};

struct pthread_rwlockattr_t_ {
  int pshared;
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


#if defined(_MSC_VER) && !defined(__cplusplus)
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
#define SE_WARNING              0x02
#define SE_ERROR                0x03

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
#define EXCEPTION_PTW32_SERVICES	\
     MAKE_SOFTWARE_EXCEPTION( SE_ERROR, \
			      PTW32_SERVICES_FACILITY, \
			      PTW32_SERVICES_ERROR )

#define PTW32_SERVICES_FACILITY		0xBAD
#define PTW32_SERVICES_ERROR	       	0xDEED

#endif /* _MSC_VER */

/*
 * Services available through EXCEPTION_PTW32_SERVICES
 * and also used [as parameters to ptw32_throw()] as
 * generic exception selectors.
 */
#define PTW32_EPS_CANCEL       0
#define PTW32_EPS_EXIT         1


/* Function pointer to TryEnterCriticalSection if it exists; otherwise NULL */
extern BOOL (WINAPI *ptw32_try_enter_critical_section)(LPCRITICAL_SECTION);

/* Declared in global.c */
extern int ptw32_processInitialized;
extern pthread_key_t ptw32_selfThreadKey;
extern pthread_key_t ptw32_cleanupKey;
extern CRITICAL_SECTION ptw32_mutex_test_init_lock;
extern CRITICAL_SECTION ptw32_cond_test_init_lock;
extern CRITICAL_SECTION ptw32_rwlock_test_init_lock;

/* Declared in misc.c */
#ifdef NEED_CALLOC
#define calloc(n, s) ptw32_calloc(n, s)
void *ptw32_calloc(size_t n, size_t s);
#endif

/* Declared in private.c */
void ptw32_throw(DWORD exception);

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
int ptw32_processInitialize (void);

void ptw32_processTerminate (void);

void ptw32_threadDestroy (pthread_t tid);

void ptw32_cleanupStack (void);

pthread_t ptw32_new (void);

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
unsigned PT_STDCALL
#else
void
#endif
ptw32_threadStart (ThreadParms * threadParms);

void ptw32_callUserDestroyRoutines (pthread_t thread);

int ptw32_tkAssocCreate (ThreadKeyAssoc ** assocP,
			    pthread_t thread,
			    pthread_key_t key);

void ptw32_tkAssocDestroy (ThreadKeyAssoc * assoc);

int ptw32_sem_timedwait (sem_t * sem,
			    const struct timespec * abstime);

#ifdef NEED_SEM
void ptw32_decrease_semaphore(sem_t * sem);
BOOL ptw32_increase_semaphore(sem_t * sem,
                                 unsigned int n);
#endif /* NEED_SEM */

#ifdef __cplusplus
}
#endif /* __cplusplus */


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

#endif /* __CYGWIN32__ || __CYGWIN__ || NEED_CREATETHREAD*/


#endif /* _IMPLEMENT_H */


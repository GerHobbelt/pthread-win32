/* This is the POSIX thread API (POSIX 1003).

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* FIXME: do not include function prototypes for functions which are
   not yet implemented.  This will allow us to keep a better handle on
   where we're at. */

#ifndef _PTHREADS_H
#define _PTHREADS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif /* HAVE_SIGNAL_H */

#ifndef SIG_BLOCK
#define SIG_BLOCK 0
#endif /* SIG_BLOCK */

#ifndef SIG_UNBLOCK 
#define SIG_BLOCK 1
#endif /* SIG_UNBLOCK */

#ifndef SIG_SETMASK
#define SIG_SETMASK 2
#endif /* SIG_SETMASK */

#define PTHREAD_THREADS_MAX 128
#define PTHREAD_STACK_MIN   65535

/* Convert these to defined when implemented. */
#define _POSIX_THREAD_ATTR_STACKSIZE
#ifdef _POSIX_THREAD_ATTR_STACKADDR
#undef _POSIX_THREAD_ATTR_STACKADDR
#endif

/* Thread scheduling policies */

#define SCHED_OTHER 0
#define SCHED_FIFO  1
#define SCHED_RR    2

#define SCHED_MIN   SCHED_OTHER
#define SCHED_MAX   SCHED_RR

/* Cancelation return value.
   This value must be neither NULL nor the value of any
   pointer to an object in memory. */
#define PTHREAD_CANCELED            ((void *) 1)

typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;
typedef DWORD pthread_key_t;


/*                                      Related constants */
typedef struct {
  long valid;

#ifdef _POSIX_THREAD_ATTR_STACKSIZE
  size_t stacksize;                  /* PTHREAD_STACK_MIN */
#endif

  int detachedstate;                 /* PTHREAD_CREATE_DETACHED
					PTHREAD_CREATE_JOINABLE */

#ifdef HAVE_SIGSET_T
  sigset_t sigmask;
#endif /* HAVE_SIGSET_T */

  int priority;

} pthread_attr_t;

/* I don't know why this structure isn't in some kind of namespace.
   According to my O'Reilly book, this is what this struct is
   called. */
struct sched_param {
  int sched_priority;
}

typedef struct {
  enum { SIGNAL, BROADCAST, NUM_EVENTS };

  /* Signal and broadcast event HANDLEs. */
  HANDLE events[NUM_EVENTS];

  /* Count of the number of waiters. */
  unsigned waiters_count;
  
  /* Serialize access to waiters_count. */
  pthread_mutex_t waiters_count_lock;
} pthread_cond_t;

typedef struct { void * dummy; } pthread_condattr_t;
typedef struct { void * dummy; } pthread_mutexattr_t;

typedef struct {
  unsigned short flag;
  pthread_mutex_t lock;
} pthread_once_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int pthread_atfork (void (*prepare)(void),
		    void (*parent)(void),
		    void (*child)(void));

int pthread_create(pthread_t *thread,
		   const pthread_attr_t *attr,
		   void * (*start_routine) (void *),
		   void * arg);

void pthread_exit(void *value);

pthread_t pthread_self(void);

int pthread_equal(pthread_t t1, pthread_t t2);

int pthread_join(pthread_t thread, void ** valueptr);

int pthread_detach(pthread_t thread);

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

/* Functions for manipulating thread attribute objects. */

int pthread_attr_init(pthread_attr_t *attr);

int pthread_attr_destroy(pthread_attr_t *attr);

int pthread_attr_setstacksize(pthread_attr_t *attr,
			      size_t stacksize);

int pthread_attr_getstacksize(const pthread_attr_t *attr,
			      size_t *stacksize);

int pthread_attr_setstackaddr(pthread_attr_t *attr,
			      void *stackaddr);

int pthread_attr_getstackaddr(const pthread_attr_t *attr,
			      void **stackaddr);

int pthread_attr_getschedparam(const pthread_attr_t *attr,
			       struct sched_param *param);

int pthread_attr_setschedparam(pthread_attr_t *attr,
			       const struct sched_param *param);

int pthread_attr_getdetachstate(const pthread_attr_t *attr,
				int *detachstate);

int pthread_attr_setdetachstate(pthread_attr_t *attr,
				int detachstate);
  
int pthread_setschedparam(pthread_t thread,
			  int policy,
			  const struct sched_param *param);

int pthread_getschedparam(pthread_t thread,
			  int *policy,
			  struct sched_param *param);

int sched_get_priority_min(int policy);

int sched_get_priority_max(int policy);

int pthread_setcancelstate(int state,
			   int *oldstate);

int pthread_setcanceltype(int type,
			  int *oldtype);

/* Functions for manipulating cond. var. attribute objects. */

int pthread_condattr_init(pthread_condattr_t *attr);

int pthread_condattr_setpshared(pthread_condattr_t *attr,
			       int pshared);

int pthread_condattr_getpshared(pthread_condattr_t *attr,
				int *pshared);

int pthread_condattr_destroy(pthread_condattr_t *attr);

/* Functions for manipulating mutex attribute objects. */

int pthread_mutexattr_init(pthread_mutexattr_t *attr);

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr,
				 int pshared);

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr,
				 int *pshared);

/* Primitives for condition variables. */

int pthread_cond_init(pthread_cond_t *cv,
		      const pthread_condattr_t *attr);

int pthread_cond_broadcast(pthread_cond_t *cv);

int pthread_cond_signal(pthread_cond_t *cv);

int pthread_cond_timedwait(pthread_cond_t *cv,
			   pthread_mutex_t *mutex,
			   const struct timespec *abstime);

int pthread_cond_wait(pthread_cond_t *cv,
		      pthread_mutex_t *mutex);

int pthread_cond_destroy(pthread_cond_t *cv);

/* Primitives for mutexes. */

int pthread_mutex_init(pthread_mutex_t *mutex,
		       pthread_mutex_attr_t *attr);

int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex);

int pthread_mutex_trylock(pthread_mutex_t *mutex);

int pthread_mutex_unlock(pthread_mutex_t *mutex);

/* Primitives for thread-specific data (TSD). */

int pthread_key_create(pthread_key_t *key,
		       void (*destructor)(void *));

int pthread_setspecific(pthread_key_t key, void *value);

void *pthread_getspecific(pthread_key_t key);

int pthread_key_delete(pthread_key_t key);

/* Signal handling. */

int pthread_sigmask(int how,
		    const sigset_t *set,
		    sigset_t *oset);

/* Thread cancelation functions. */

void pthread_testcancel(void);

int pthread_cancel(pthread_t thread);

#ifdef __cplusplus
}
#endif /* __cplusplus */

extern const int _pthread_create_joinable;
extern const int _pthread_create_detached;

/* Cancelability attributes */
extern const int _pthread_cancel_enable;
extern const int _pthread_cancel_disable;

extern const int _pthread_cancel_asynchronous;
extern const int _pthread_cancel_deferred;

#define PTHREAD_CREATE_JOINABLE     _pthread_create_joinable
#define PTHREAD_CREATE_DETACHED     _pthread_create_detached

/* Cancelability attributes */
#define PTHREAD_CANCEL_ENABLE       _pthread_cancel_enable
#define PTHREAD_CANCEL_DISABLE      _pthread_cancel_disable

#define PTHREAD_CANCEL_ASYNCHRONOUS _pthread_cancel_asynchronous
#define PTHREAD_CANCEL_DEFERRED     _pthread_cancel_deferred


/* The following #defines implement POSIX cleanup handlers.
   The standard requires that these functions be used as statements and
   be used pairwise in the same scope. The standard suggests that, in C, they
   may be implemented as macros starting and ending the same block.

   POSIX requires that applications observe scoping requirements, but
   doesn't say if the implemention must enforce them. The macros below
   partially enforce scope but can lead to compile or runtime errors. */

enum {
  _PTHREAD_HANDLER_POP_LIFO,
  _PTHREAD_HANDLER_POP_FIFO
};

enum {
  _PTHREAD_CLEANUP_STACK,
  _PTHREAD_DESTRUCTOR_STACK,
  _PTHREAD_FORKPREPARE_STACK,
  _PTHREAD_FORKPARENT_STACK,
  _PTHREAD_FORKCHILD_STACK
};

#ifdef pthread_cleanup_push
#undef pthread_cleanup_push
#endif

#define pthread_cleanup_push(routine, arg) \
{ \
  (void ) _pthread_handler_push(_PTHREAD_CLEANUP_STACK, \
				_PTHREAD_HANDLER_POP_LIFO, routine, arg);

#ifdef pthread_cleanup_pop
#undef pthread_cleanup_pop
#endif

#define pthread_cleanup_pop(execute) \
  _pthread_handler_pop(_PTHREAD_CLEANUP_STACK, execute);\
}

#endif /* _PTHREADS_H */

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

typedef HANDLE pthread_t;
typedef CRITICAL_SECTION pthread_mutex_t;

typedef struct {
  enum { SIGNAL, BROADCAST, NUM_EVENTS };

  /* Signal and broadcast event HANDLEs. */
  HANDLE events[NUM_EVENTS];

  /* Count of the number of waiters. */
  unsigned waiters_count;
  
  /* Serialize access to waiters_count_. */
  pthread_mutex_t waiters_count_lock;
} pthread_cond_t;

typedef struct { void * ptr; } pthread_condattr_t;
typedef struct { void * ptr; } pthread_mutexattr_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int pthread_create(pthread_t *thread,
		   const pthread_attr_t *attr,
		   void * (*start_routine) (void *),
		   void * arg);

void pthread_exit(void *value);

pthread_t pthread_self(void);

int pthread_equal(pthread_t t1, pthread_t t2);

int pthread_join(pthread_t thread, void ** valueptr);

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

/* These functions cannot be implemented in terms of the Win32 API.
   Fortunately they are optional.  Their implementation just returns
   the correct error number. */

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr,
				  int protocol);

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
				  int *protocol);

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr,
				      int prioceiling);

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
				     int *ceiling);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PTHREADS_H */

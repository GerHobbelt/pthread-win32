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
typedef DWORD pthread_key_t;

typedef struct {
  enum { SIGNAL, BROADCAST, NUM_EVENTS };

  /* Signal and broadcast event HANDLEs. */
  HANDLE events[NUM_EVENTS];

  /* Count of the number of waiters. */
  unsigned waiters_count;
  
  /* Serialize access to waiters_count. */
  pthread_mutex_t waiters_count_lock;
} pthread_cond_t;

typedef struct { void * ptr; } pthread_condattr_t;
typedef struct { void * ptr; } pthread_mutexattr_t;

typedef struct {
  unsigned short flag;
  pthread_mutex_t lock;
} pthread_once_t;

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

/* Primitives for thread-specific data (TSD). */

int pthread_key_create(pthread_key_t *key,
		       void (*destructor)(void *));

int pthread_setspecific(pthread_key_t key, void *value);

void *pthread_getspecific(pthread_key_t key);

int pthread_key_delete(pthread_key_t key);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* Below here goes all internal definitions required by this implementation
   of pthreads for Win32.
 */

/* An element in the thread table. */
typedef struct _pthread_threads_thread _pthread_threads_thread_t;
struct _pthread_threads_thread {
   pthread_t                  thread;
  _pthread_attr_t            *attr;
};

/* _PTHREAD_BUILD_DLL must only be defined if we are building the DLL. */
#ifndef _PTHREADS_BUILD_DLL
/* Static global data that must be static within the application
   but not the DLL.
 */
pthread_mutex_t _pthread_count_mutex = PTHREAD_MUTEX_INITIALIZER;
DWORD _pthread_threads_count = 0;
_pthread_threads_thread_t _pthread_threads_table[PTHREAD_THREADS_MAX];
unsigned short _pthread_once_flag;
pthread_mutex_t _pthread_once_lock = PTHREAD_MUTEX_INITIALIZER;
#else
extern pthread_mutex_t _pthread_count_mutex;
extern DWORD _pthread_threads_count;
extern _pthread_threads_thread_t _pthread_threads_table[];
extern unsigned short _pthread_once_flag;
pthread_mutex_t _pthread_once_lock;
#endif

/* End of application static data */

#endif /* _PTHREADS_H */

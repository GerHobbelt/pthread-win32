/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

int
pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	       void * (*start_routine) (void *), void * arg)
{
  /* Call Win32 CreateThread.
     Map attributes as correctly as possible.
     The passed in attr structure will be modified by this routine
     to reflect any default values used. This is POSIX semantics.
  */
  HANDLE   handle = NULL;
  unsigned flags;
  unsigned stack;
  void *   security = NULL;

  /* FIXME: This needs to be moved into process space. 
     Perhaps into a structure that contains all
     per thread info that is Win32 thread specific but
     not visible from the pthreads API, and
     accessible through HANDLE (or pthread_t).
   */
  SECURITY_ATTRIBUTES security_attr;
  DWORD  threadID;
  int t;
  int ret = 0; /* Success unless otherwise set */
  char * privmem;
  pthread_attr_t * attr_copy;
  _pthread_cleanup_stack_t * cleanup_stack;

  /* Use and modify attr_copy. Only after we've succeeded in creating the
     new thread can we modify any passed-in structures.

     To save time we use one malloc() to get all of our heap space and
     then allocate it further.
   */

  if (NULL == 
      (privmem = (char) malloc(RND_SIZEOF(pthread_attr_t) +
			       RND_SIZEOF(_pthread_cleanup_stack_t)))) {
    return EAGAIN;
  }

  attr_copy = (pthread_attr_t *) privmem;

  /* Force cleanup_stack to start at a DWORD boundary within privmem.
   */
  cleanup_stack = 
    (_pthread_cleanup_stack_t *) &privmem[RND_SIZEOF(pthread_attr_t)];

  (void) memcpy(attr_copy, attr);

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);

  if (_pthread_threads_count < PTHREAD_THREADS_MAX) {
    switch (attr)
      {
      case NULL:
	/* Use POSIX default attributes */
	stack = attr_copy->stacksize = PTHREAD_STACK_MIN;
	break;
      default:
	/* Map attributes */
	if (attr_copy.stacksize != NULL)
	  stack = (DWORD) attr_copy->stacksize;
	else
	  stack = attr_copy->stacksize = PTHREAD_STACK_MIN;
	break;
      }

    flags = 1; /* Start suspended and resume at the last moment to avoid
		  race conditions, ie. where a thread may enquire it's
		  attributes before we finish storing them away.
		*/

    handle = (HANDLE) _beginthreadex(security,
				   stack,
				   (unsigned (_stdcall *)(void *)) start_routine,
				   arg,
				   flags,
				   &threadID);

    if (handle != NULL) {
      _pthread_threads_count++;

      /* The hash table works as follows:
	 hash into the table,
	 if the slot is occupied then start single stepping from there
	 until we find an available slot.
       */
      t = _PTHREAD_HASH_INDEX(handle);
      while ((_pthread_threads_table[t])->thread != NULL) {
	t++;

	if (t == PTHREAD_THREADS_MAX)
	  t = 0; /* Wrap to the top of the table. */
      }

      if ((_pthread_threads_table[t])->thread != NULL) {
	/* INTERNAL ERROR */
      } else {
	(_pthread_threads_table[t])->thread = handle;
	(_pthread_threads_table[t])->attr = attr_copy;
	(_pthread_threads_table[t])->cleanupstack = cleanup_stack;
      }
    } else {
      ret = EAGAIN;
    }
  } else {
    ret = EAGAIN;
  }

  /* Let others in as soon as possible. */
  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */

  if (ret == 0) {
    *thread = (pthread_t) handle;
    (void) memcpy(attr, attr_copy);

    /* POSIX threads are always running after creation.
     */
    ResumeThread(handle);
  } else {
    free(privmem);
  }

  return ret;
}

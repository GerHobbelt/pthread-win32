/*
 * exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
pthread_exit (void *value_ptr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *
      * PARAMETERS
      *      value_ptr
      *              a generic data value (i.e. not the address of a value)
      *
      *
      * DESCRIPTION
      *      This function terminates the calling thread, returning
      *      the value 'value_ptr' to any joining thread.
      *      NOTE: thread should be joinable.
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  _pthread_callUserDestroyRoutines(pthread_getspecific(_pthread_selfThreadKey));

  _endthreadex ((unsigned) value_ptr);

  return (0);

}				/* pthread_exit */

/* </JEB> */


#if 0 /* Pre Bossom */

void
_pthread_vacuum(void)
{
  /* Run all the handlers. */
  _pthread_handler_pop_all(_PTHREAD_CLEANUP_STACK, 
			   _PTHREAD_HANDLER_EXECUTE);

  /* Pop any atfork handlers without executing them. */
  _pthread_handler_pop_all(_PTHREAD_FORKPREPARE_STACK, 
			   _PTHREAD_HANDLER_NOEXECUTE);

  _pthread_handler_pop_all(_PTHREAD_FORKPARENT_STACK, 
			   _PTHREAD_HANDLER_NOEXECUTE);

  _pthread_handler_pop_all(_PTHREAD_FORKCHILD_STACK,
			   _PTHREAD_HANDLER_NOEXECUTE);
}

void
_pthread_exit(pthread_t thread, void * value, int return_code)
{
  int detachstate;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  /* Copy value into the thread entry so it can be given
     to any joining threads. */
  thread->joinvalueptr = value;

  pthread_mutex_unlock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  _pthread_vacuum();

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  /* Remove the thread entry on exit only if the thread is detached
     AND there are no waiting joins. Otherwise the thread entry will
     be deleted by the last waiting pthread_join() after this thread
     has terminated. */

  if (pthread_attr_getdetachstate(&thread->attr, &detachstate) == 0 
      && detachstate == PTHREAD_CREATE_DETACHED
      && thread->join_count == 0)
    {
      (void) _pthread_delete_thread(thread);
    }

  pthread_mutex_unlock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  _endthreadex(return_code);
}

void
pthread_exit(void * value)
{
  _pthread_exit(pthread_self(), value, 0);
}

#endif /* Pre Bossom */

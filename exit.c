/*
 * exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
 */

#include "pthread.h"
#include "implement.h"

void
_pthread_vacuum(void)
{
  /* Run all the handlers. */
  _pthread_handler_pop_all(_PTHREAD_CLEANUP_STACK, 
			   _PTHREAD_HANDLER_EXECUTE);

  _pthread_handler_pop_all(_PTHREAD_DESTRUCTOR_STACK, 
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
_pthread_exit(void * value, int return_code)
{
  _pthread_threads_thread_t * us = _PTHREAD_THIS;

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  /* Copy value into the thread entry so it can be given
     to any joining threads. */
  us->joinvalueptr = value;

  pthread_mutex_lock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  _pthread_vacuum();

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_table_mutex);

  /* Remove the thread entry on exit only if pthread_detach() was
     called AND there are no waiting joins. Otherwise the thread entry
     will be deleted by the last waiting pthread_join() after this
     thread has terminated. */

  if (us->detach == TRUE
      && us->join_count == 0)
    {
      _pthread_delete_thread_entry(us);
    }

  pthread_mutex_lock(&_pthread_table_mutex);
  /* END CRITICAL SECTION */

  _endthreadex(return_code);
}

void
pthread_exit(void * value)
{
  _pthread_exit(value, 0);
}

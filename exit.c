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
pthread_exit(void * value)
{
  _pthread_threads_thread_t * this;

  this = _PTHREAD_THIS;

  /* Copy value into the thread entry so it can be given
     to any joining threads. */
  if (this->joinvalueptr != NULL)
    {
      this->joinvalueptr = value;
    }

  /* Teleport back to _pthread_start_call() to cleanup and exit. */
  longjmp(this->call.env, 1);
}

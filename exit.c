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

void
_pthread_vacuum(void)
{
  /* This function can be called from pthread_exit(), or from
     _pthread_start_call() in which case cleanupstack should be
     empty but destructorstack still needs to be run. */
  _pthread_threads_thread_t * this;

  this = *_PTHREAD_THIS;

  /* Run all the handlers. */
  _pthread_handler_pop_all(&(this->cleanupstack), _PTHREAD_HANDLER_EXECUTE);
  _pthread_handler_pop_all(&(this->destructorstack), _PTHREAD_HANDLER_EXECUTE);

  /* Pop any atfork handlers to free storage. */
  _pthread_handler_pop_all(&(this->forkprepare), _PTHREAD_HANDLER_NOEXECUTE);
  _pthread_handler_pop_all(&(this->forkparent), _PTHREAD_HANDLER_NOEXECUTE);
  _pthread_handler_pop_all(&(this->forkchild), _PTHREAD_HANDLER_NOEXECUTE);

  _pthread_delete_thread_entry(NULL);
}

void
pthread_exit(void * value)
{
  _pthread_vacuum();
  _endthreadex((DWORD) value);
}

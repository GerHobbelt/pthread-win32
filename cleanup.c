/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

#include <errno.h>

#include <malloc.h>
#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

_pthread_cleanup_t *
_pthread_pop_cleanup (int execute)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function pops the most recently pushed cleanup
      *      handler. If execute is nonzero, then the cleanup handler
      *      is executed if non-null.
      *
      * PARAMETERS
      *      execute
      *              if nonzero, execute the cleanup handler
      *
      *
      * DESCRIPTION
      *      This function pops the most recently pushed cleanup
      *      handler. If execute is nonzero, then the cleanup handler
      *      is executed if non-null.
      *      NOTE: specify 'execute' as nonzero to avoid duplication
      *                of common cleanup code.
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  _pthread_cleanup_t *cleanup;
  
  cleanup = pthread_getspecific (_pthread_cleanupKey);

  if (cleanup != NULL)
    {
      if (execute && (cleanup->routine != NULL))
        {

#ifdef _WIN32

          __try
          {
            /*
             * Run the caller's cleanup routine.
             */
            (*cleanup->routine) (cleanup->arg);
          }
          __except (EXCEPTION_EXECUTE_HANDLER)
          {
            /*
             * A system unexpected exception had occurred
             * running the user's cleanup routine.
             * We get control back within this block.
             */
          }
        }

#else

      /*
       * Run the caller's cleanup routine.
       */
      (*cleanup->routine) (cleanup->arg);

#endif /* _WIN32 */

      pthread_setspecific (_pthread_cleanupKey, cleanup->prev);
    }

  return (cleanup);

}                               /* _pthread_pop_cleanup */


void
_pthread_push_cleanup (_pthread_cleanup_t * cleanup,
		      void (*routine) (void *),
		      void *arg)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function pushes a new cleanup handler onto the thread's stack
      *      of cleanup handlers. Each cleanup handler pushed onto the stack is
      *      popped and invoked with the argument 'arg' when
      *              a) the thread exits by calling 'pthread_exit',
      *              b) when the thread acts on a cancellation request,
      *              c) or when the thrad calls pthread_cleanup_pop with a nonzero
      *                 'execute' argument
      *
      * PARAMETERS
      *      cleanup
      *              a pointer to an instance of pthread_cleanup_t,
      *
      *      routine
      *              pointer to a cleanup handler,
      *
      *      arg
      *              parameter to be passed to the cleanup handler
      *
      *
      * DESCRIPTION
      *      This function pushes a new cleanup handler onto the thread's stack
      *      of cleanup handlers. Each cleanup handler pushed onto the stack is
      *      popped and invoked with the argument 'arg' when
      *              a) the thread exits by calling 'pthread_exit',
      *              b) when the thread acts on a cancellation request,
      *              c) or when the thrad calls pthread_cleanup_pop with a nonzero
      *                 'execute' argument
      *      NOTE: pthread_push_cleanup, pthread_pop_cleanup must be paired
      *                in the same lexical scope.
      *
      * RESULTS
      *              pthread_cleanup_t *
      *                              pointer to the previous cleanup
      *
      * ------------------------------------------------------
      */
{
  cleanup->routine = routine;
  cleanup->arg = arg;
  cleanup->prev = pthread_getspecific (_pthread_cleanupKey);

  pthread_setspecific (_pthread_cleanupKey, (void *) cleanup);

}                               /* _pthread_push_cleanup */

/* </JEB> */


#if 0 /* Pre Bossom */

int
_pthread_handler_push(int stack,
		      int poporder,
		      void (*routine)(void *), 
		      void *arg)
{
  /* Place the new handler into the list so that handlers are
     popped off in the order given by poporder. */
  _pthread_handler_node_t * new_handler;
  _pthread_handler_node_t * next;
  _pthread_handler_node_t ** stacktop;

  stacktop = _PTHREAD_STACK(stack);

  new_handler = 
    (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new_handler == NULL)
    {
      return 0; /* NOMEM */
    }

  new_handler->routine = routine;
  new_handler->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new_handler->next = *stacktop;
      *stacktop = new_handler;
    }
  else
    {
      /* Add the new node to the end of the list. */
      new_handler->next = NULL;

      if (*stacktop == NULL)
	{
	  *stacktop = new_handler;
	}
      else
	{
	  next = *stacktop;

	  while (next->next != NULL)
	    {
	      next = next->next;
	    }

	  next->next = new_handler;
	}
    }
  return 0;
}

void
_pthread_handler_pop(int stack, int execute)
{
  _pthread_handler_node_t ** stacktop;
  _pthread_handler_node_t * next;
  void (* func)(void *);
  void * arg;

  stacktop = _PTHREAD_STACK(stack);

  if (*stacktop != NULL)
    {
      func = (*stacktop)->routine;
      arg = (*stacktop)->arg;
      next = (*stacktop)->next;

      free(*stacktop);
      *stacktop = next;

      if (execute != 0 && func != NULL)
	{
	  (void) func(arg);
	}
    }
}

void
_pthread_handler_pop_all(int stack, int execute)
{
  /* Pop and possibly run all handlers on the given stack. */
  _pthread_handler_node_t ** stacktop;
  _pthread_handler_node_t * next;
  void (* func)(void *);
  void * arg;

  stacktop = _PTHREAD_STACK(stack);

  while (*stacktop != NULL)
    {
      func = (*stacktop)->routine;
      arg = (*stacktop)->arg;
      next = (*stacktop)->next;

      free(*stacktop);
      *stacktop = next;

      if (execute != 0 && func != NULL)
	{
	  (void) func(arg);
	}
    }
}

/* Run destructors for all non-NULL key values for the calling thread.
 */
void
_pthread_destructor_run_all()
{
  _pthread_tsd_key_t * key;
  int count;
  int dirty;

  /* This threads private keys */
  key = _pthread_tsd_key_table;

  /* Stop destructor execution at a finite time. POSIX allows us
     to ignore this if we like, even at the risk of an infinite loop.

     FIXME: We don't know when to stop yet.
   */
  for (count = 0; count < PTHREAD_DESTRUCTOR_ITERATIONS; count++)
    {
      int k;
      void * arg;

      dirty = 0;

      /* Loop through all keys. */
      for (k = 0; k < _POSIX_THREAD_KEYS_MAX; k++)
	{
	  /* CRITICAL SECTION */
	  pthread_mutex_lock(&_pthread_tsd_mutex);

	  switch (key->status)
	    {
	    case _PTHREAD_TSD_KEY_INUSE:
	      arg = pthread_getspecific((pthread_key_t) k);

	      if (arg != NULL && key->destructor != NULL)
		{
		  /* The destructor must be called with the mutex off. */
		  pthread_mutex_unlock(&_pthread_tsd_mutex);
		  /* END CRITICAL SECTION */

		  /* FIXME: Is the destructor supposed to set the key value
		     to NULL? How is this done when arg is the key value, not
		     a pointer to it? For now we assume that the destructor
		     always succeeds.
		     */
		  (void) (key->destructor)(arg);

		  /* CRITICAL SECTION */
		  pthread_mutex_lock(&_pthread_tsd_mutex);

		  pthread_setspecific((pthread_key_t) k, NULL);
#if 0
		  /* Only needed if we don't assume the destructor
		     always succeeds.
		     */
		  dirty = 1;
#endif
		}
	      break;

	    case _PTHREAD_TSD_KEY_DELETED:
	      key->status = _PTHREAD_TSD_KEY_INUSE;
	      pthread_setspecific((pthread_key_t) k, NULL);

	      if (key->in_use <= 0)
		{
		  /* This is the last thread to use this
		     deleted key. It can now be made available
		     for re-use.
		   */
		  key->status = _PTHREAD_TSD_KEY_REUSE;
		  _pthread_key_reuse[_pthread_key_reuse_top++] = k;
		}
	      else
		{
		  key->status = _PTHREAD_TSD_KEY_DELETED;
		}
	      break;

	    default:
	      break;
	    }

	  pthread_mutex_unlock(&_pthread_tsd_mutex);
	  /* END CRITICAL SECTION */

	  key++;
	}

      if (!dirty)
	break;
    }
}

#endif /* Pre Bossom */

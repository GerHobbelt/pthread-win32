/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

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



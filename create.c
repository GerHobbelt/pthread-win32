/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
pthread_create (pthread_t * tid,
		const pthread_attr_t * attr,
		void *(*start) (void *),
		void *arg)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function creates a thread running the start function,
      *      passing it the parameter value, 'arg'.
      *
      * PARAMETERS
      *      tid
      *              pointer to an instance of pthread_t
      *
      *      attr
      *              optional pointer to an instance of pthread_attr_t
      *
      *      start
      *              pointer to the starting routine for the new thread
      *
      *      arg
      *              optional parameter passed to 'start'
      *
      *
      * DESCRIPTION
      *      This function creates a thread running the start function,
      *      passing it the parameter value, 'arg'. The 'attr'
      *      argument specifies optional creation attributes.
      *      The thread is identity of the new thread is returned
      *      as 'tid'
      *
      * RESULTS
      *              0               successfully created thread,
      *              EINVAL          attr invalid,
      *              EAGAIN          insufficient resources.
      *
      * ------------------------------------------------------
      */
{
  pthread_t thread;
  int result = EAGAIN;
  int run = TRUE;
  ThreadParms *parms;
  long stackSize;

  if ((thread = (pthread_t) calloc (1, sizeof (*thread))) ==
      NULL)
    {
      goto FAIL0;
    }
  thread->cancelEvent =
    CreateEvent (
		  0,
		  (int) TRUE,	/* manualReset  */
		  (int) FALSE,	/* setSignaled  */
		  NULL);

  if (thread->cancelEvent == NULL)
    {
      goto FAIL0;
    }

  if ((parms = (ThreadParms *) malloc (sizeof (*parms))) ==
      NULL)
    {
      goto FAIL0;
    }

  parms->tid = thread;
  parms->start = start;
  parms->arg = arg;

  if (attr != NULL && *attr != NULL)
    {
      stackSize = (*attr)->stacksize;
      thread->detachState = (*attr)->detachstate;

#if HAVE_SIGSET_T

      thread->sigmask = (*attr)->sigmask;

#endif /* HAVE_SIGSET_T */

    }
  else
    {
      /*
       * Default stackSize
       */
      stackSize = 0;
    }

  thread->state = run
    ? PThreadStateInitial
    : PThreadStateSuspended;

  thread->keys = NULL;
  thread->threadH = (HANDLE)
    _beginthreadex (
		     (void *) NULL,	/* No security info             */
		     (unsigned) stackSize,	/* default stack size   */
		     (unsigned (PT_STDCALL *) (void *)) _pthread_threadStart,
		     parms,
		     (unsigned) run ? 0 : CREATE_SUSPENDED,
		     (unsigned *) &(thread->thread));

  result = (thread->threadH != 0) ? 0 : EAGAIN;

  /*
   * Fall Through Intentionally
   */

  /*
   * ------------
   * Failure Code
   * ------------
   */

FAIL0:
  if (result != 0)
    {

      _pthread_threadDestroy (thread);
      thread = NULL;

      if (parms != NULL)
	{
	  free (parms);
	}
    }
  *tid = thread;

  return (result);

}				/* pthread_create */

/* </JEB> */


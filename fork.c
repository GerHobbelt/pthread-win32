/*
 * fork.c
 *
 * Description:
 * Implementation of fork() for POSIX threads.
 */

#include "pthread.h"
#include "implement.h"

int
pthread_atfork(void (*prepare)(void),
	       void (*parent)(void),
	       void (*child)(void))
{
  /* Push handlers (unless NULL) onto their respective stacks. 

     Local implementation semantics:
     If we get an ENOMEM at any time in here then ALL handlers
     (including those from previous pthread_atfork() calls) will be
     popped off each of the three atfork stacks before we return. */

  int ret = 0;

  if (prepare != NULL)
    {
      /* Push prepare. */
      if (_pthread_handler_push(_PTHREAD_FORKPREPARE_STACK,
				_PTHREAD_HANDLER_POP_FIFO,
				(void (*prepare)(void *)), NULL) == ENOMEM)
	{
	  ret = ENOMEM;
	}
    }

  if (parent != NULL &&
      ret != ENOMEM)
    {
      /* Push parent. */
      if (_pthread_handler_push(_PTHREAD_FORKPARENT_STACK,
				_PTHREAD_HANDLER_POP_LIFO,
				(void (*parent)(void *)), NULL) == ENOMEM)
	{
	  ret = ENOMEM;
	}
    }

  if (child != NULL &&
      ret != ENOMEM)
    {
      /* Push child. */
      if (_pthread_handler_push(_PTHREAD_FORKCHILD_STACK,
				_PTHREAD_HANDLER_POP_LIFO,
				(void (*child)(void *)), arg) == ENOMEM)
	{
	  ret = ENOMEM;
	}
    }

  if (ret == ENOMEM)
    {
      /* Pop all handlers without executing them before we return
	 the error. */
      _pthread_handler_pop_all(_PTHREAD_FORKPREPARE_STACK,
			       _PTHREAD_HANDLER_NOEXECUTE);

      _pthread_handler_pop_all(_PTHREAD_FORKPARENT_STACK,
			       _PTHREAD_HANDLER_NOEXECUTE);

      _pthread_handler_pop_all(_PTHREAD_FORKCHILD_STACK,
			       _PTHREAD_HANDLER_NOEXECUTE);
    }

  return ret;
}

/* It looks like the GNU linker is capable of selecting this version of
   fork() over a version provided in more primitive libraries further down
   the linker command line. */

#if HAVE_PID_T && HAVE_FORK

pid_t
fork()
{
  pid_t pid;

  /* Pop prepare handlers here. */
  _pthread_handler_pop_all(_PTHREAD_FORKPREPARE_STACK,
			   _PTHREAD_HANDLER_EXECUTE);

  /* Now call the real fork(). */

  if ((pid = _fork()) > 0)
    {
      /* PARENT */
      /* Clear the child handler stack. */
      _pthread_handler_pop_all(_PTHREAD_FORKCHILD_STACK,
			       _PTHREAD_HANDLER_NOEXECUTE);

      /* Pop parent handlers and execute them. */
      _pthread_handler_pop_all(_PTHREAD_FORKPARENT_STACK,
			       _PTHREAD_HANDLER_EXECUTE);

      /* At this point all three atfork stacks are empty. */
      return pid;
    }
  else
    {
      /* CHILD */
      /* Clear the parent handler stack. */
      _pthread_handler_pop_all(_PTHREAD_FORKPARENT_STACK,
			       _PTHREAD_HANDLER_NOEXECUTE);

      /* Pop child handlers and execute them. */
      _pthread_handler_pop_all(_PTHREAD_FORKCHILD_STACK,
			       _PTHREAD_HANDLER_EXECUTE);

      /* At this point all three atfork stacks are empty. */

      /* Terminate all threads except pthread_self() using
	 pthread_cancel(). */
      _pthread_cancel_all_not_self();
      _pthread_join_all_not_self();

      return 0;
    }

  /* Not reached. */
}

#endif /* HAVE_PID_T && HAVE_FORK */

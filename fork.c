/*
 * fork.c
 *
 * Description:
 * Implementation of fork() for POSIX threads.
 */

int
pthread_atfork(void (*prepare)(void),
	       void (*parent)(void),
	       void (*child)(void))
{
  /* Push handlers (unless NULL) onto their respective stacks. */

  if (prepare != NULL)
    {
      /* Push prepare. */
      /* If push fails, return ENOMEM. */
    }

  if (parent != NULL)
    {
      /* Push parent. */
      /* If push fails, return ENOMEM. */
    }

  if (child != NULL)
    {
      /* Push child. */
      /* If push fails, return ENOMEM. */
    }

  /* Everything is okay. */
  return 0;
}

/* It looks like the GNU linker is capable of selecting this version of
   fork() over a version provided in more primitive libraries further down
   the linker command line. */

pid_t
fork()
{
  pid_t pid;

  /* Pop prepare handlers here. */

  /* Now call Cygwin32's fork(). */

  if ((pid = _fork()) > 0)
    {
      /* Pop parent handlers. */
      return pid;
    }
  else
    {
      /* Pop child handlers. */
      /* Terminate all threads except pthread_self() using
	 pthread_cancel(). */
      return 0;
    }

  /* Not reached. */
}

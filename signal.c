/*
 * signal.c
 *
 * Description:
 * POSIX thread-aware signal functions.
 */

#include "pthread.h"
#include "implement.h"

int
pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
  pthread_t thread = pthread_self();

  /* Validate the `how' argument.*/
  if (set != NULL)
    {
      switch (how)
	{
	case SIG_BLOCK:
	  break;
	case SIG_UNBLOCK:
	  break;
	case SIG_SETMASK:
	  break;
	default:
	  /* Invalid `how' argument. */
	  return EINVAL;
	}
    }

  /* Copy the old mask before modifying it. */
  if (oset != NULL)
    {
      memcpy(oset, &(thread->attr.sigmask), sizeof(sigset_t));
    }

  if (set != NULL)
    {
      int i;
	
      /* FIXME: this code assumes that sigmask is an even multiple of
	 the size of a long integer. */ 
         
      unsigned long *src = (long *) set;
      unsigned long *dest = &(thread->attr.sigmask);

      switch (how)
	{
	case SIG_BLOCK:
	  for (i = 0; i < (sizeof(sigset_t) / sizeof(unsigned long)); i++)
	    {
	      /* OR the bit field longword-wise. */
	      *src++ |= *dest++;
	    }
	  break;
	case SIG_UNBLOCK:
	  for (i = 0; i < (sizeof(sigset_t) / sizeof(unsigned long)); i++)
	    {
	      /* XOR the bitfield longword-wise. */
	      *src++ ^= *dest++;
	    }
	case SIG_SETMASK:
	  /* Replace the whole sigmask. */
	  memcpy(&(thread->attr.sigmask), set, sizeof(sigset_t));
	  break;
	}
    }

  return 0;
}

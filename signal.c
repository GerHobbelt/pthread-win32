/*
 * signal.c
 *
 * Description:
 * POSIX thread-aware signal functions.
 */

#include "pthread.h"

int
pthread_sigmask(int how, const sigset_t *set, sigset_t *oset)
{
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
      memcpy(oset, this->attr->sigmask, sizeof(sigset_t));
    }

  if (set != NULL)
    {
      _pthread_threads_thread_t * us = _PTHREAD_THIS;
      int i;
	
      /* FIXME: this code assumes that sigmask is an even multiple of
	 the size of a long integer. */ 
         
      unsigned long *src = set;
      unsigned long *dest = us->attr.sigmask;

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
	case SIG_SET:
	  /* Replace the whole sigmask. */
	  memcpy(us->attr.sigmask, set, sizeof(sigset_t));
	  break;
	}
    }

  return 0;
}

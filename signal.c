/*
 * signal.c
 *
 * Description:
 * Thread-aware signal functions.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

/* errno.h or a replacement file is included by pthread.h */
//#include <errno.h>

#include "pthread.h"
#include "implement.h"

#if HAVE_SIGSET_T
int
pthread_sigmask(int how, sigset_t const *set, sigset_t *oset)
{
  pthread_t thread = pthread_self();

  if (thread == NULL)
    {
      return ENOENT;
    }

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
      memcpy(oset, &(thread->sigmask), sizeof(sigset_t));
    }

  if (set != NULL)
    {
      unsigned int i;
	
      /* FIXME: this code assumes that sigmask is an even multiple of
	 the size of a long integer. */ 
         
      unsigned long *src = (unsigned long const *) set;
      unsigned long *dest = (unsigned long *) &(thread->sigmask);

      switch (how)
	{
	case SIG_BLOCK:
	  for (i = 0; i < (sizeof(sigset_t) / sizeof(unsigned long)); i++)
	    {
	      /* OR the bit field longword-wise. */
	      *dest++ |= *src++;
	    }
	  break;
	case SIG_UNBLOCK:
	  for (i = 0; i < (sizeof(sigset_t) / sizeof(unsigned long)); i++)
	    {
	      /* XOR the bitfield longword-wise. */
	      *dest++ ^= *src++;
	    }
	case SIG_SETMASK:
	  /* Replace the whole sigmask. */
	  memcpy(&(thread->sigmask), set, sizeof(sigset_t));
	  break;
	}
    }

  return 0;
}
#endif /* HAVE_SIGSET_T */

/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

#include "pthread.h"
#include "implement.h"

void
_pthread_handler_push(_pthread_handler_node_t ** stacktop,
		      int poporder,
		      void (*routine)(void *), 
		      void *arg)
{
  /* Place the new handler into the list so that handlers are
     popped off in the order given by poporder. */
  _pthread_handler_node_t * new;
  _pthread_handler_node_t * next;

  new = (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new == NULL)
    {
      /* FIXME: INTERNAL ERROR */
    }

  new->routine = routine;
  new->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new->next = *stacktop;
      stacktop = next;
    }
  else
    {
      /* Add the new node to the end of the list. */
      new->next = NULL;

      if (*stacktop == NULL)
	{
	  *stacktop = new;
	}
      else
	{
	  next = *stacktop;
	  while (next != NULL)
	    {
	      next = next->next;
	    }
	  next = new;
	}
    }
}

void
_pthread_handler_pop(_pthread_handler_node_t ** stacktop,
		     int execute)
{
  _pthread_handler_node_t * handler = *stacktop;

  if (handler != NULL)
    {
      void (* func)(void *) = handler->routine;
      void * arg = handler->arg;

      *stacktop = handler->next;

      free(handler);

      if (execute != 0 && func != NULL)
	{
	  (void) func(arg);
	}
    }
}

void
_pthread_handler_pop_all(_pthread_handler_node_t ** stacktop, 
			 int execute)
{
  /* Pop and run all handlers on the given stack. */
  while (*stacktop != NULL)
    {
      _pthread_handler_pop(stacktop, execute);
    }
}

/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

#include "pthread.h"
#include "implement.h"

int
_pthread_handler_push(int stack,
		      int poporder,
		      void (*routine)(void *), 
		      void *arg)
{
  /* Place the new handler into the list so that handlers are
     popped off in the order given by poporder. */
  _pthread_handler_node_t * new;
  _pthread_handler_node_t * next;
  _pthread_handler_node_t ** stacktop;

  stacktop = _PTHREAD_STACK(stack);

  new = (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new == NULL)
    {
      return ENOMEM;
    }

  new->routine = routine;
  new->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new->next = *stacktop;
      *stacktop = next;
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

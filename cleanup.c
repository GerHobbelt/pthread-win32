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

int
_pthread_handler_push(int stack,
		      int poporder,
		      void (*routine)(void *), 
		      void *arg)
{
  /* Place the new handler into the list so that handlers are
     popped off in the order given by poporder. */
  _pthread_handler_node_t * new_thread;
  _pthread_handler_node_t * next;
  _pthread_handler_node_t ** stacktop;

  stacktop = _PTHREAD_STACK(stack);

  new_thread = 
    (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new_thread == NULL)
    {
      return 0; /* NOMEM */
    }

  new_thread->routine = routine;
  new_thread->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new_thread->next = *stacktop;
      *stacktop = next;
    }
  else
    {
      /* Add the new node to the end of the list. */
      new_thread->next = NULL;

      if (*stacktop == NULL)
	{
	  *stacktop = new_thread;
	}
      else
	{
	  next = *stacktop;

	  while (next != NULL)
	    {
	      next = next->next;
	    }

	  next = new_thread;
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


int
_pthread_destructor_push(void (* routine)(void *), pthread_key_t key)
{
  return _pthread_handler_push(_PTHREAD_DESTRUCTOR_STACK, 
			       _PTHREAD_HANDLER_POP_LIFO,
			       routine, 
			       (void *) key);
}


/* Remove all of the destructors associated with the key. */
void
_pthread_destructor_pop(pthread_key_t key)
{
  _pthread_handler_node_t ** head;
  _pthread_handler_node_t * current;
  _pthread_handler_node_t * next;

  head = _PTHREAD_STACK(_PTHREAD_DESTRUCTOR_STACK);
  current = *head;

  while (current != NULL)
    {
      next = current->next;

      /* The destructors associated key is in current->arg. */
      if (current->arg == (void *) key)
	{
	  if (current == *head)
	    {
	      *head = next;
	    }
	  free(current);
	}
      current = next;
    }
}


/* Run destructors for all non-NULL key values.

   FIXME: Currently we only run the destructors on the calling
   thread's key values. The way I interpret POSIX semantics is that,
   for each key that the calling thread has a destructor for, we need
   to look at the key values of every thread and run the destructor on
   it if the key value is non-NULL.

   The question is: how do we access the key associated values which
   are private to other threads?

 */
void
_pthread_destructor_pop_all()
{
  _pthread_handler_node_t ** head;
  _pthread_handler_node_t * current;
  _pthread_handler_node_t * next;
  void (* func)(void *);
  void * arg;
  int count;

  head = _PTHREAD_STACK(_PTHREAD_DESTRUCTOR_STACK);

  /* Stop destructor execution at a finite time. POSIX allows us
     to ignore this if we like, even at the risk of an infinite loop.
   */
  for (count = 0; count < PTHREAD_DESTRUCTOR_ITERATIONS; count++)
    {
      /* Loop through all destructors for this thread. */
      while (current != NULL)
	{
	  func = current->routine;

	  /* Get the key value using the key which is in current->arg. */
	  arg = pthread_getspecific((int) current->arg);

	  next = current->next;

	  /* If the key value is non-NULL run the destructor, otherwise
	     unlink it from the list.
	   */
	  if (arg != NULL)
	    {
	      if (func != NULL)
		{
		  (void) func(arg);
		}
	    }
	  else
	    {
	      if (current == *head)
		{
		  *head = next;
		}
	      free(current);
	    }
	  current = next;
	}
    }

  /* Free the destructor list even if we still have non-NULL key values. */
  while (*head != NULL)
    {
      next = (*head)->next;
      free(*head);
      *head = next;
    }
}

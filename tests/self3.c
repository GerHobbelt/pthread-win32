#include <pthread.h>
/* Hack. Peer into implementation details. */
#include <implement.h>
#include <assert.h>
#include <stdio.h>

void *
entry(void * arg)
{
  /* Like systems such as HP-UX, we can't print the value of the thread ID
     because it's not an integral type. Instead, we'll poke our noses into
     the pthread_t structure and dump a useful internal value. This is
     ordinarily bad, m'kay? */

  pthread_t t = pthread_self();
  printf("thread no. %d has id %lx\n", (int) arg, t->win32handle); 
  return 0; 
}

int
main()
{
  int rc;
  pthread_t t[2];

  rc = pthread_create(&t[0], NULL, entry, (void *) 1);
  assert(rc == 0);

  rc = pthread_create(&t[1], NULL, entry, (void *) 2);
  assert(rc == 0);

  Sleep(2000);
  return 0;
}

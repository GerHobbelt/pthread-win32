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
  printf("my thread is %lx\n", t->win32handle); 
  return arg;
}

int
main()
{
  int rc;
  pthread_t t;

  rc = pthread_create(&t, NULL, entry, NULL);
  assert(rc == 0);

  Sleep(2000);
  return 0;
}

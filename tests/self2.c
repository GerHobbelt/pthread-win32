#include <pthread.h>
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
  printf("my thread is %lx\n", t->threadH); 
  return arg;
}

int
main()
{
  int rc;
  pthread_t t;

  if (pthread_create(&t, NULL, entry, NULL) != 0)
    {
      return 1;
    }

  Sleep(2000);

  /* Success. */
  return 0;
}

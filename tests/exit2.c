/*
 * Test for pthread_exit().
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --------------------------------------------------------------------------
 *
 * Depends on API functions:
 *	pthread_create()
 *	pthread_exit()
 */

#include "test.h"

void *
func(void * arg)
{
  ptrdiff_t failed = (ptrdiff_t) arg;
  /* int failed = (int) arg; */

  pthread_exit(arg);

  /* Never reached. */
  /*
   * Trick gcc compiler into not issuing a warning here
   */
  assert(failed - (ptrdiff_t) arg);

  return NULL;
}

int
main(int argc, char * argv[])
{
  pthread_t t;

  assert(pthread_create(&t, NULL, func, (void *) NULL) == 0);

  Sleep(100);

  return 0;
}

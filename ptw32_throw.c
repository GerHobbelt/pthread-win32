/*
 * ptw32_throw.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
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
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "pthread.h"
#include "implement.h"

#if defined(PTW32_CLEANUP_C)
# include <setjmp.h>
#endif

/*
 * ptw32_throw
 *
 * All cancelled and explicitly exited POSIX threads go through
 * here. This routine knows how to exit both POSIX initiated threads and
 * 'implicit' POSIX threads for each of the possible language modes (C,
 * C++, and SEH).
 */
void
ptw32_throw (DWORD exception)
{
  /*
   * Don't use pthread_self() to avoid creating an implicit POSIX thread handle
   * unnecessarily.
   */
  ptw32_thread_t * sp = (ptw32_thread_t *) pthread_getspecific (ptw32_selfThreadKey);

#if defined(PTW32_CLEANUP_SEH)
  DWORD exceptionInformation[3];
#endif

  sp->state = PThreadStateExiting;

  if (exception !=  PTW32_EPS_CANCEL && exception !=  PTW32_EPS_EXIT)
    {
      /* Should never enter here */
      exit (1);
    }

  if (NULL == sp || sp->implicit)
    {
      /*
       * We're inside a non-POSIX initialised Win32 thread
       * so there is no point to jump or throw back to. Just do an
       * explicit thread exit here after cleaning up POSIX
       * residue (i.e. cleanup handlers, POSIX thread handle etc).
       */
#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
      unsigned int exitCode = 0;

      switch (exception)
        {
      	  case PTW32_EPS_CANCEL:
      		exitCode = (unsigned int)(size_t) PTHREAD_CANCELED;
      		break;
      	  case PTW32_EPS_EXIT:
      		if (NULL != sp)
      		  {
      			exitCode = (unsigned int)(size_t) sp->exitStatus;
      		  }
      		break;
        }
#endif

#if defined(PTW32_STATIC_LIB)

      pthread_win32_thread_detach_np ();

#endif

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)
      _endthreadex (exitCode);
#else
      _endthread ();
#endif

    }

#if defined(PTW32_CLEANUP_SEH)


  exceptionInformation[0] = (DWORD) (exception);
  exceptionInformation[1] = (DWORD) (0);
  exceptionInformation[2] = (DWORD) (0);

  RaiseException (EXCEPTION_PTW32_SERVICES, 0, 3, (ULONG_PTR *) exceptionInformation);

#else /* PTW32_CLEANUP_SEH */

#if defined(PTW32_CLEANUP_C)

  ptw32_pop_cleanup_all (1);
  longjmp (sp->start_mark, exception);

#else /* PTW32_CLEANUP_C */

#if defined(PTW32_CLEANUP_CXX)

  switch (exception)
    {
    case PTW32_EPS_CANCEL:
      throw ptw32_exception_cancel ();
      break;
    case PTW32_EPS_EXIT:
      throw ptw32_exception_exit ();
      break;
    }

#else

#error ERROR [__FILE__, line __LINE__]: Cleanup type undefined.

#endif /* PTW32_CLEANUP_CXX */

#endif /* PTW32_CLEANUP_C */

#endif /* PTW32_CLEANUP_SEH */

  /* Never reached */
}


void
ptw32_pop_cleanup_all (int execute)
{
  while (NULL != ptw32_pop_cleanup (execute))
    {
    }
}


DWORD
ptw32_get_exception_services_code (void)
{
#if defined(PTW32_CLEANUP_SEH)

  return EXCEPTION_PTW32_SERVICES;

#else

  return (DWORD)0;

#endif
}

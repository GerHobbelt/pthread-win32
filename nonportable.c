/*
 * nonportable.c
 *
 * Description:
 * This translation unit implements non-portable thread functions.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
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

#include "pthread.h"
#include "implement.h"

/*
 * pthread_mutexattr_setkind_np()
 */
int pthread_mutexattr_setkind_np(pthread_mutexattr_t * attr, int kind)
{
  return pthread_mutexattr_settype( attr, kind );
}


/*
 * pthread_mutexattr_getkind_np()
 */
int pthread_mutexattr_getkind_np(pthread_mutexattr_t * attr, int *kind)
{
  return pthread_mutexattr_gettype( attr, kind );
}


/*
 * pthread_getw32threadhandle_np()
 *
 * Returns the win32 thread handle that the POSIX
 * thread "thread" is running as.
 *
 * Applications can use the win32 handle to set
 * win32 specific attributes of the thread.
 */
HANDLE
pthread_getw32threadhandle_np(pthread_t thread)
{
  return (thread != NULL) ? (thread->threadH) : 0;
}


/*
 * Provide pthread_delay_np posix function for NT
 *
 * DESCRIPTION
 *
 *       This routine causes a thread to delay execution for a specific period of time.
 *       This period ends at the current time plus the specified interval. The routine
 *       will not return before the end of the period is reached, but may return an
 *       arbitrary amount of time after the period has gone by. This can be due to
 *       system load, thread priorities, and system timer granularity. 
 *
 *       Specifying an interval of zero (0) seconds and zero (0) nanoseconds is
 *       allowed and can be used to force the thread to give up the processor or to
 *       deliver a pending cancelation request. 
 *
 *       The timespec structure contains the following two fields: 
 *
 *            tv_sec is an integer number of seconds. 
 *            tv_nsec is an integer number of nanoseconds. 
 *
 *  Return Values
 *
 *  If an error condition occurs, this routine returns an integer value indicating
 *  the type of error. Possible return values are as follows: 
 *
 *  0 
 *           Successful completion.
 *  [EINVAL] 
 *           The value specified by interval is invalid. 
 *
 * Example
 *
 * The following code segment would wait for 5 and 1/2 seconds
 *
 *  struct timespec tsWait;
 *  int      intRC;
 *
 *  tsWait.tv_sec  = 5;
 *  tsWait.tv_nsec = 500000000L;
 *  intRC = pthread_delay_np(&tsWait);
 */
int
pthread_delay_np (struct timespec * interval)
{
  DWORD  wait_time, secs_in_millisecs, millisecs;

  /*
   * We are a cancelation point.
   */
  pthread_testcancel();

  if (interval->tv_sec < 0 || interval->tv_nsec < 0)
    {
      return (EINVAL);
    }

  secs_in_millisecs = interval->tv_sec * 1000L;           /* convert secs to millisecs */

  /*
   * Pedantically, we're ensuring that we don't return before the time is up,
   * even by a fraction of a millisecond.
   */
  millisecs = (interval->tv_nsec + 999999L) / 1000000L;    /* convert nanosecs to millisecs */

  wait_time         = secs_in_millisecs + millisecs;

  Sleep(wait_time);

  pthread_testcancel();

  return (0);
}


/*
 * pthread_num_processors_np()
 *
 * Get the number of CPUs available to the process.
 */
int
pthread_num_processors_np(void)
{
  int count;

  if ( ptw32_getprocessors(& count) != 0 )
    {
      count = 1;
    }

  return (count);
}


/*
 * Handle to kernel32.dll 
 */
static HINSTANCE ptw32_h_kernel32;

BOOL
pthread_win32_process_attach_np ()
{
  BOOL result = TRUE;

  result = ptw32_processInitialize ();
#ifdef _UWIN
  pthread_count++;
#endif

#ifndef TEST_ICE

  /*
   * Load KERNEL32 and try to get address of InterlockedCompareExchange
   */
  ptw32_h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));

  ptw32_interlocked_compare_exchange =
    (PTW32_INTERLOCKED_LONG (WINAPI *)(PTW32_INTERLOCKED_LPLONG, PTW32_INTERLOCKED_LONG, PTW32_INTERLOCKED_LONG))
#if defined(NEED_UNICODE_CONSTS)
    GetProcAddress(ptw32_h_kernel32,
                   (const TCHAR *)TEXT("InterlockedCompareExchange"));
#else
    GetProcAddress(ptw32_h_kernel32,
                   (LPCSTR) "InterlockedCompareExchange");
#endif

  if (ptw32_interlocked_compare_exchange == NULL)
    {
      ptw32_interlocked_compare_exchange = ptw32_InterlockedCompareExchange;

      /*
       * If InterlockedCompareExchange is not being used, then free
       * the kernel32.dll handle now, rather than leaving it until
       * DLL_PROCESS_DETACH.
       *
       * Note: this is not a pedantic exercise in freeing unused
       * resources!  It is a work-around for a bug in Windows 95
       * (see microsoft knowledge base article, Q187684) which
       * does Bad Things when FreeLibrary is called within
       * the DLL_PROCESS_DETACH code, in certain situations.
       * Since w95 just happens to be a platform which does not
       * provide InterlockedCompareExchange, the bug will be
       * effortlessly avoided.
       */
      (void) FreeLibrary(ptw32_h_kernel32);
      ptw32_h_kernel32 = 0;
    }

#else /* TEST_ICE */

  ptw32_interlocked_compare_exchange = ptw32_InterlockedCompareExchange;

#endif /* TEST_ICE */

  return result;
}

BOOL
pthread_win32_process_detach_np ()
{
  if (ptw32_processInitialized)
    {
      pthread_t self = (pthread_t) pthread_getspecific (ptw32_selfThreadKey);

      /*
       * Detached threads have their resources automatically
       * cleaned up upon exit (others must be 'joined').
       */
      if (self != NULL &&
          self->detachState == PTHREAD_CREATE_DETACHED)
        {
          pthread_setspecific (ptw32_selfThreadKey, NULL);
          ptw32_threadDestroy (self);
        }

      /*
       * The DLL is being unmapped into the process's address space
       */
      ptw32_processTerminate ();

      if (ptw32_h_kernel32)
        {
           (void) FreeLibrary(ptw32_h_kernel32);
        }
    }

  return TRUE;
}

BOOL
pthread_win32_thread_attach_np ()
{
  return TRUE;
}

BOOL
pthread_win32_thread_detach_np ()
{
  if (ptw32_processInitialized)
    {
       pthread_t self = (pthread_t) pthread_getspecific (ptw32_selfThreadKey);

       /*
        * Detached threads have their resources automatically
        * cleaned up upon exit (others must be 'joined').
        */
       if (self != NULL &&
           self->detachState == PTHREAD_CREATE_DETACHED)
         {
           pthread_setspecific (ptw32_selfThreadKey, NULL);
           ptw32_threadDestroy (self);
         }
    }

  return TRUE;
}

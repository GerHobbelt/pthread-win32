/*
 * ptw32_relmillisecs.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2016, Pthreads4w contributors
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
#if !defined(NEED_FTIME)
#include <sys/timeb.h>
#endif

static const int64_t NANOSEC_PER_SEC = 1000000000;
static const int64_t NANOSEC_PER_MILLISEC = 1000000;
static const int64_t MILLISEC_PER_SEC = 1000;

#if defined (__PTW32_BUILD_INLINED)
INLINE
#endif /*  __PTW32_BUILD_INLINED */
DWORD
__ptw32_relmillisecs (const struct timespec * abstime)
{
  DWORD milliseconds;
  int64_t tmpAbsMilliseconds;
  int64_t tmpAbsNanoseconds;
  int64_t tmpCurrMilliseconds;
  int64_t tmpCurrNanoseconds;

#if defined(NEED_FTIME)
  struct timespec currSysTime;
  FILETIME ft;
#else /* ! NEED_FTIME */
#if ( defined(_MSC_VER) && _MSC_VER >= 1300 ) /* MSVC7+ */ || \
    ( defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601 )
  struct __timeb64 currSysTime;
#else
  struct _timeb currSysTime;
#endif
#endif /* NEED_FTIME */


  /*
   * Calculate timeout as milliseconds from current system time.
   */

  /*
   * subtract current system time from abstime in a way that checks
   * that abstime is never in the past, or is never equivalent to the
   * defined INFINITE value (0xFFFFFFFF).
   *
   * Assume all integers are unsigned, i.e. cannot test if less than 0.
   */
  tmpAbsMilliseconds =  (int64_t)abstime->tv_sec * MILLISEC_PER_SEC;
  tmpAbsMilliseconds += ((int64_t)abstime->tv_nsec + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC;
  tmpAbsNanoseconds = (int64_t)abstime->tv_nsec + ((int64_t)abstime->tv_sec * NANOSEC_PER_SEC);

  /* get current system time */

#if defined(NEED_FTIME)

# if defined(WINCE)

  SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
# else
  GetSystemTimeAsFileTime(&ft);
# endif

  __ptw32_filetime_to_timespec(&ft, &currSysTime);

  tmpCurrMilliseconds = (int64_t)currSysTime.tv_sec * MILLISEC_PER_SEC;
  tmpCurrMilliseconds += ((int64_t)currSysTime.tv_nsec + (NANOSEC_PER_MILLISEC/2))
			   / NANOSEC_PER_MILLISEC;
  tmpCurrNanoseconds = (int64_t)currSysTime->tv_nsec + ((int64_t)currSysTime->tv_sec * NANOSEC_PER_SEC);

#else /* ! NEED_FTIME */

#if defined(_MSC_VER) && _MSC_VER >= 1400  /* MSVC8+ */
  _ftime64_s(&currSysTime);
#elif ( defined(_MSC_VER) && _MSC_VER >= 1300 ) /* MSVC7+ */ || \
      ( defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601 )
  _ftime64(&currSysTime);
#else
  _ftime(&currSysTime);
#endif

  tmpCurrMilliseconds = (int64_t) currSysTime.time * MILLISEC_PER_SEC;
  tmpCurrMilliseconds += (int64_t) currSysTime.millitm;
  tmpCurrNanoseconds = tmpCurrMilliseconds * NANOSEC_PER_MILLISEC;

#endif /* NEED_FTIME */

  if (tmpAbsMilliseconds > tmpCurrMilliseconds)
    {
      milliseconds = (DWORD) (tmpAbsMilliseconds - tmpCurrMilliseconds);
      if (milliseconds == INFINITE)
        {
          /* Timeouts must be finite */
          milliseconds--;
        }
    }
  else
    {
      /* The abstime given is in the past */
      milliseconds = 0;
    }

  if (milliseconds == 0 && tmpAbsNanoseconds > tmpCurrNanoseconds) {
     /*
      * millisecond granularity was too small to represent the wait time.
      * return the minimum time in milliseconds.
      */
     milliseconds = 1;
 }

  return milliseconds;
}


/*
 * Return the first parameter "abstime" modified to represent the current system time.
 * If "relative" is not NULL it represents an interval to add to "abstime".
 */

struct timespec *
pthread_win32_getabstime_np (struct timespec * abstime, const struct timespec * relative)
{
  int64_t sec;
  int64_t nsec;

#if defined(NEED_FTIME)
  struct timespec currSysTime;
  FILETIME ft;
#else /* ! NEED_FTIME */
#if ( defined(_MSC_VER) && _MSC_VER >= 1300 ) /* MSVC7+ */ || \
    ( defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601 )
  struct __timeb64 currSysTime;
#else
  struct _timeb currSysTime;
#endif
#endif /* NEED_FTIME */

  /* get current system time */

#if defined(NEED_FTIME)

# if defined(WINCE)

  SYSTEMTIME st;
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
# else
  GetSystemTimeAsFileTime(&ft);
# endif

  __ptw32_filetime_to_timespec(&ft, &currSysTime);

  sec = currSysTime.tv_sec;
  nsec = currSysTime.tv_nsec;

#else /* ! NEED_FTIME */

#if defined(_MSC_VER) && _MSC_VER >= 1400  /* MSVC8+ */
  _ftime64_s(&currSysTime);
#elif ( defined(_MSC_VER) && _MSC_VER >= 1300 ) /* MSVC7+ */ || \
      ( defined(__MINGW32__) && __MSVCRT_VERSION__ >= 0x0601 )
  _ftime64(&currSysTime);
#else
  _ftime(&currSysTime);
#endif

  sec = currSysTime.time;
  nsec = currSysTime.millitm * NANOSEC_PER_MILLISEC;

#endif /* NEED_FTIME */

  if (NULL != relative)
    {
      nsec += relative->tv_nsec;
      if (nsec >= NANOSEC_PER_SEC)
	{
	  sec++;
	  nsec -= NANOSEC_PER_SEC;
	}
      sec += relative->tv_sec;
    }

  abstime->tv_sec = (time_t) sec;
  abstime->tv_nsec = (long) nsec;

  return abstime;
}

/*
 * pthread_barrier_wait.c
 *
 * Description:
 * This translation unit implements barrier primitives.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 *
 *      Contact Email: rpj@callisto.canberra.edu.au
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef PTW32_STATIC_LIB

#if defined(__MINGW32__) || defined(_MSC_VER)

#include "pthread.h"
#include "implement.h"

static void on_process_init(void)
{
    pthread_win32_process_attach_np ();
}

static void on_process_exit(void)
{
    pthread_win32_thread_detach_np  ();
    pthread_win32_process_detach_np ();
}

#if defined(__MINGW32__)
# define attribute_section(a) __attribute__((section(a)))
#elif defined(_MSC_VER)
# define attribute_section(a) __pragma(section(a,long,read)); __declspec(allocate(a))
#endif

attribute_section(".ctors") void *gcc_ctor = on_process_init;
attribute_section(".dtors") void *gcc_dtor = on_process_exit;

attribute_section(".CRT$XCU") void *msc_ctor = on_process_init;
attribute_section(".CRT$XPU") void *msc_dtor = on_process_exit;

#else
# warning ==================================================================
# warning STATIC LINK LIBRARY BUILD
# warning Auto Initialization/Termination of PTHREADS-WIN32
# warning
# warning This compiler is not supported (yet) for auto initialization
# warning when pthreads-win32 is statically linked. Any linked code must
# warning call pthread_win32_process_attach_np() and
# warning pthread_win32_process_detach_np() explicitly.
# warning See README.NONPORTABLE for the description of those routines.
# warning ==================================================================
#endif /* defined(__MINGW32__) || defined(_MSC_VER) */

#warning ==================================================================
#warning STATIC LINK LIBRARY BUILD
#warning Auto-reclaiming of POSIX resources acquired by Windows threads
#warning
#warning If code linked with this library includes Windows threads that
#warning explicitly interact with POSIX threads, i.e. by calling POSIX API
#warning routines, then those threads will acquire POSIX resources and
#warning should call pthread_win32_thread_detach_np() on thread exit (see
#warning README.NONPORTABLE), especially if the linked code depends on
#warning reclaimed resources or the running of POSIX TSD destructors.
#warning NOTE 1: Otherwise this will occur only when the process exits.
#warning NOTE 2: Threads created via pthread_create() always auto-reclaim.
#warning ==================================================================

#endif /* PTW32_STATIC_LIB */

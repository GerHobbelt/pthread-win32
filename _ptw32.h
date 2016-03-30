/*
 * Module: _ptw32.h
 *
 * Purpose:
 *      Pthreads-win32 internal macros, to be shared by other headers
 *      comprising the pthreads-win32 package.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2012, 2016, Pthreads-win32 contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
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
 *
 */
#ifndef __PTW32_H
#define __PTW32_H

#if defined __GNUC__
# pragma GCC system_header
# if ! defined __declspec
#  error "Please upgrade your GNU compiler to one that supports __declspec."
# endif
#endif

#if defined (__cplusplus)
# define __PTW32_BEGIN_C_DECLS  extern "C" {
# define __PTW32_END_C_DECLS    }
#else
# define __PTW32_BEGIN_C_DECLS
# define __PTW32_END_C_DECLS
#endif

#if defined (PTW32_STATIC_LIB) && _MSC_VER >= 1400
# undef PTW32_STATIC_LIB
# define PTW32_STATIC_TLSLIB
#endif

/* When building the library, you should define PTW32_BUILD so that
 * the variables/functions are exported correctly. When using the library,
 * do NOT define PTW32_BUILD, and then the variables/functions will
 * be imported correctly.
 *
 * FIXME: Used defined feature test macros, such as PTW32_STATIC_LIB, (and
 * maybe even PTW32_BUILD), should be renamed with one initial underscore;
 * internally defined macros, such as PTW32_DLLPORT, should be renamed with
 * two initial underscores ... perhaps __PTW32_DECLSPEC is nicer anyway?
 */
#if defined PTW32_STATIC_LIB || defined PTW32_STATIC_TLSLIB
# define PTW32_DLLPORT

#elif defined PTW32_BUILD
# define PTW32_DLLPORT __declspec (dllexport)
#else
# define PTW32_DLLPORT /*__declspec (dllimport)*/
#endif

#ifndef PTW32_CDECL
/* FIXME: another internal macro; should have two initial underscores;
 * Nominally, we prefer to use __cdecl calling convention for all our
 * functions, but we map it through this macro alias to facilitate the
 * possible choice of alternatives; for example:
 */
# ifdef _OPEN_WATCOM_SOURCE
  /* The Open Watcom C/C++ compiler uses a non-standard default calling
   * convention, (similar to __fastcall), which passes function arguments
   * in registers, unless the __cdecl convention is explicitly specified
   * in exposed function prototypes.
   *
   * Our preference is to specify the __cdecl convention for all calls,
   * even though this could slow Watcom code down slightly.  If you know
   * that the Watcom compiler will be used to build both the DLL and your
   * application, then you may #define _OPEN_WATCOM_SOURCE, so disabling
   * the forced specification of __cdecl for all function declarations;
   * remember that this must be defined consistently, for both the DLL
   * build, and the application build.
   */
#  define PTW32_CDECL
# else
#  define PTW32_CDECL __cdecl
# endif
#endif

#endif	/* !__PTW32_H */

# configure.ac
# --------------------------------------------------------------------------
#
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
#
AC_INIT([pthreads-win32],[git])
AC_CONFIG_HEADERS([config.h])

# Checks for build tools.
#
AC_PROG_CC
AC_PROG_CXX
AC_CHECK_TOOLS([AR],[ar],[ar])
AC_CHECK_TOOLS([DLLTOOL],[dlltool],[dlltool])
AC_CHECK_TOOLS([OBJDUMP],[objdump],[objdump])
AC_CHECK_TOOLS([RC],[windres],[windres])
AC_PROG_RANLIB

# Autoconf doesn't normally guard config.h, but we prefer it so;
# this is also a convenient place to force HAVE_C_INLINE, because
# AC_C_INLINE makes it available, even if the build compiler does
# not normally support it.
#
AH_TOP(dnl
[#ifndef __PTW32_CONFIG_H]
[#define __PTW32_CONFIG_H]dnl
)
# FIXME: AC_C_INLINE defines 'inline' to work with any C compiler,
# whether or not it supports inline code expansion, but it does NOT
# define HAVE_C_INLINE; can we use this standard autoconf feature,
# without also needing to define HAVE_C_INLINE?
#
AC_C_INLINE
AH_BOTTOM([#define HAVE_C_INLINE])
AH_BOTTOM([#endif])

# Checks for data types and structures.
#
PTW32_AC_CHECK_TYPEDEF([mode_t])
PTW32_AC_CHECK_TYPEDEF([sigset_t],[signal.h])
PTW32_AC_CHECK_TYPEDEF([struct timespec],[time.h])

# Checks for __cdecl functions.
#
PTW32_AC_NEED_ERRNO
PTW32_AC_NEED_FUNC([NEED_CALLOC],[calloc])
PTW32_AC_NEED_FUNC([NEED_FTIME],[ftime])
PTW32_AC_NEED_FUNC([NEED_CREATETHREAD],[_beginthreadex])
PTW32_AC_CHECK_CPU_AFFINITY

# WinAPI functions need a full argument list for detection.
#
PTW32_AC_NEED_WINAPI_FUNC([DuplicateHandle],[NULL,NULL,NULL,NULL,0,0,0])

# Checks for installation tools.
#
AC_PROG_MKDIR_P
AC_PROG_INSTALL

# Build system generation, as configured.
#
AC_CONFIG_FILES([GNUmakefile tests/GNUmakefile])
AC_OUTPUT

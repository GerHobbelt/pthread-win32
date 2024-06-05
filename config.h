/* config.h  */

#ifndef PTW32_CONFIG_H
#define PTW32_CONFIG_H

/*********************************************************************
 * Defaults: see target specific redefinitions below.
 *********************************************************************/

 /* We're building the pthreads-win32 library */
#define PTW32_BUILD

/* CPU affinity */
#define HAVE_CPU_AFFINITY

/* Do we know about the C type sigset_t? */
#undef HAVE_SIGSET_T

/* Define if you have the <signal.h> header file.  */
#undef HAVE_SIGNAL_H

/* Define if you have the Borland TASM32 or compatible assembler.  */
#undef HAVE_TASM32

/* Define if you don't have Win32 DuplicateHandle. (eg. WinCE) */
#undef NEED_DUPLICATEHANDLE

/* Define if you don't have Win32 _beginthreadex. (eg. WinCE) */
#undef NEED_CREATETHREAD

/* Define if you don't have Win32 errno. (eg. WinCE) */
#undef NEED_ERRNO

/* Define if you don't have Win32 calloc. (eg. WinCE)  */
#undef NEED_CALLOC

/* Define if you don't have Win32 semaphores. (eg. WinCE 2.1 or earlier)  */
#undef NEED_SEM

/* Define if you need to convert string parameters to unicode. (eg. WinCE)  */
#undef NEED_UNICODE_CONSTS

/* Define if your C (not C++) compiler supports "inline" functions. */
#undef HAVE_C_INLINE

/* Do we know about type mode_t? */
#undef HAVE_MODE_T

/*
 * Define if GCC has atomic builtins, i.e. __sync_* intrinsics
 * __sync_lock_* is implemented in mingw32 gcc 4.5.2 at least
 * so this define does not turn those on or off. If you get an
 * error from __sync_lock* then consider upgrading your gcc.
 */
#undef HAVE_GCC_ATOMIC_BUILTINS

 /* Define if you have the timespec struct */
#undef HAVE_STRUCT_TIMESPEC

/* Define if you don't have the GetProcessAffinityMask() */
#undef NEED_PROCESS_AFFINITY_MASK

/* Define if your version of Windows TLSGetValue() clears WSALastError
 * and calling SetLastError() isn't enough restore it. You'll also need to
 * link against wsock32.lib (or libwsock32.a for MinGW).
 */
#undef RETAIN_WSALASTERROR

 /*
 # ----------------------------------------------------------------------
 # The library can be built with some alternative behaviour to better
 # facilitate development of applications on Win32 that will be ported
 # to other POSIX systems.
 #
 # Nothing described here will make the library non-compliant and strictly
 # compliant applications will not be affected in any way, but
 # applications that make assumptions that POSIX does not guarantee are
 # not strictly compliant and may fail or misbehave with some settings.
 #
 # PTW32_THREAD_ID_REUSE_INCREMENT
 # Purpose:
 # POSIX says that applications should assume that thread IDs can be
 # recycled. However, Solaris (and some other systems) use a [very large]
 # sequence number as the thread ID, which provides virtual uniqueness.
 # This provides a very high but finite level of safety for applications
 # that are not meticulous in tracking thread lifecycles e.g. applications
 # that call functions which target detached threads without some form of
 # thread exit synchronisation.
 #
 # Usage:
 # Set to any value in the range: 0 <= value < 2^wordsize.
 # Set to 0 to emulate reusable thread ID behaviour like Linux or *BSD.
 # Set to 1 for unique thread IDs like Solaris (this is the default).
 # Set to some factor of 2^wordsize to emulate smaller word size types
 # (i.e. will wrap sooner). This might be useful to emulate some embedded
 # systems.
 #
 # define PTW32_THREAD_ID_REUSE_INCREMENT 0
 #
 # ----------------------------------------------------------------------
  */
#undef PTW32_THREAD_ID_REUSE_INCREMENT


  /*********************************************************************
   * Target specific groups
   *
   * If you find that these are incorrect or incomplete please report it
   * to the pthreads-win32 maintainer. Thanks.
   *********************************************************************/
#if defined(WINCE)
#  undef  HAVE_CPU_AFFINITY
#  define NEED_DUPLICATEHANDLE
#  define NEED_CREATETHREAD
#  define NEED_ERRNO
#  define NEED_CALLOC
#  define NEED_FTIME
   /* #  define NEED_SEM */
#  define NEED_UNICODE_CONSTS
#  define NEED_PROCESS_AFFINITY_MASK
/* This may not be needed */
#  define RETAIN_WSALASTERROR
#endif

#if defined(_UWIN)
#  define HAVE_MODE_T
#  define HAVE_STRUCT_TIMESPEC
#  define HAVE_SIGNAL_H
#endif

#if defined(__GNUC__)
#  define HAVE_C_INLINE
#endif

#if defined(__MINGW64__)
#define HAVE_MODE_T
#define HAVE_STRUCT_TIMESPEC
#elif defined(__MINGW32__)
#define HAVE_MODE_T
#endif

#if defined(__BORLANDC__)
#define NEED_CREATETHREAD
#endif

#if defined(__WATCOMC__)
#endif

#if defined(__DMC__)
#define HAVE_SIGNAL_H
#define HAVE_C_INLINE
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1900
#define HAVE_STRUCT_TIMESPEC
#endif
#define __PTW32_CONFIG_MSVC7
#endif /*  __PTW32_CONFIG_H */

#if defined(__TINYC__)
#define NO_OLDNAMES 1

#ifdef HAVE_HIDDEN_VISIBILITY_ATTRIBUTE
#ifdef LIBFFI_ASM
#ifdef __APPLE__
#define FFI_HIDDEN(name) .private_extern name
#else
#define FFI_HIDDEN(name) .hidden name
#endif
#else
#define FFI_HIDDEN __attribute__ ((visibility ("hidden")))
#endif
#else
#ifdef LIBFFI_ASM
#define FFI_HIDDEN(name)
#else
#define FFI_HIDDEN
#undef FFI_BAD_ABI
#endif
#endif

#ifndef FFI_WIN64
#define FFI_WIN64 1
#endif
#ifndef __ILP32__
#define __ILP32__
#endif
#ifndef FFI_UNIX64
#define FFI_UNIX64
#endif

typedef long long int64_t;
typedef unsigned long long uint64_t;
#if defined(__TINYC__) && (defined(WIN64) || defined(_WIN64))
#include <atomic.h>
#endif

#ifdef __TINY_LIBC
#undef NO_OLDNAMES

#define USE_POSIX_THREADS_WEAK 1

#ifndef EOVERFLOW
#define EOVERFLOW 132
#endif
#include <io.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define ssize_t ptrdiff_t

#define _GL_INLINE_HEADER_BEGIN
#define _GL_INLINE_HEADER_END
#define _GL_EXTERN_INLINE inline
#define _GL_ATTRIBUTE_PURE
#define _GL_ATTRIBUTE_CONST
#define _GL_UNUSED
#define _GL_ATTRIBUTE_NODISCARD
#define _GL_ATTRIBUTE_MAYBE_UNUSED
#define HAVE_LONG_LONG_INT 1
# define _GL_ATTRIBUTE_FORMAT(spec) /* empty */
#define HAVE_DECL_STRERROR_R 1
#define HAVE_DECL___ARGV 1
#define _GL_ARG_NONNULL(a)
#define _GL_ATTRIBUTE_FORMAT_PRINTF(a, b)
#define _GL_ATTRIBUTE_DEPRECATED
#define _GL_ATTRIBUTE_FALLTHROUGH
#define GNULIB_XALLOC 1
#define _GL_ATTRIBUTE_MALLOC
#define _GL_ATTRIBUTE_ALLOC_SIZE(a)
#define _GL_ATTRIBUTE_DEALLOC_FREE
#define _GL_ATTRIBUTE_DEALLOC(f, i)
#define _GL_ATTRIBUTE_RETURNS_NONNULL
#define _GL_ATTRIBUTE_COLD
#define _GL_ASYNC_SAFE
#define GNULIB_XALLOC_DIE 1
#define _GL_HAVE__STATIC_ASSERT 1
#define _Static_assert(a, b)
#define __PGI

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define MIN min
#define _GL_CMP(a, b) (a) == (b)

#define __getopt_argv_const

#ifndef _GL_INLINE
#define _GL_INLINE inline
#endif

/* should be set by including "filename.h"
#ifndef FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE
#define FILE_SYSTEM_DRIVE_PREFIX_CAN_BE_RELATIVE 1
#endif */


#define __NO_ISOCEXT
#define __MINGW_ATTRIB_DEPRECATED_SEC_WARN

#define RENAME_OPEN_FILE_WORKS 1

#define _SH_COMPAT 0x00
#define _SH_DENYRW 0x10
#define _SH_DENYWR 0x20
#define _SH_DENYRD 0x30
#define _SH_DENYNO 0x40
#define _SH_SECURE 0x80

extern char *_stpcpy(char *yydest, const char *yysrc);
extern int obstack_printf(struct obstack *obs, const char *format, ...);
extern char *_stpcpy(char *yydest, const char *yysrc);
extern int strverscmp(const char *s1, const char *s2);

#define ssize_t ptrdiff_t
#include <stdio.h>
#define fopen(n,m) _fsopen(n,m,_SH_DENYNO)
#define snprintf _snprintf
#define fseeko _fseeki64

size_t __cdecl strnlen(const char *_Str, size_t _MaxCount);
int timespec_get(struct timespec *__ts, int __base);
int getdtablesize(void);
int fcntl(int fd, int action, /* arg */...);
int integer_length(unsigned int x);
int ffs(int i);

#ifndef F_DUPFD
# define F_DUPFD 1
#endif

#ifndef F_GETFD
# define F_GETFD 2
#endif

#ifndef O_CLOEXEC
# define O_CLOEXEC 0x40000000 /* Try to not collide with system O_* flags.  */
#endif

#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC 0x40000000
/* Witness variable: 1 if gnulib defined F_DUPFD_CLOEXEC, 0 otherwise.  */
#endif

#ifndef FD_CLOEXEC
# define FD_CLOEXEC 1
#endif

#  if !GNULIB_defined_TIME_UTC
#   define TIME_UTC 1
#   define GNULIB_defined_TIME_UTC 1
#  endif
#endif /* __TINY_LIBC */

#endif

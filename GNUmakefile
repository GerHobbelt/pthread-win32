#
# --------------------------------------------------------------------------
#
#      Pthreads-win32 - POSIX Threads Library for Win32
#      Copyright(C) 1998 John E. Bossom
#      Copyright(C) 1999,2002 Pthreads-win32 contributors
# 
#      Contact Email: rpj@ise.canberra.edu.au
# 
#      The current list of contributors is contained
#      in the file CONTRIBUTORS included with the source
#      code distribution. The list can also be seen at the
#      following World Wide Web location:
#      http://sources.redhat.com/pthreads-win32/contributors.html
# 
#      This library is free software; you can redistribute it and/or
#      modify it under the terms of the GNU Lesser General Public
#      License as published by the Free Software Foundation; either
#      version 2 of the License, or (at your option) any later version.
# 
#      This library is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#      Lesser General Public License for more details.
# 
#      You should have received a copy of the GNU Lesser General Public
#      License along with this library in the file COPYING.LIB;
#      if not, write to the Free Software Foundation, Inc.,
#      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
#

DEVROOT	= C:\PTHREADS

DLLDEST	= $(DEVROOT)\DLL
LIBDEST	= $(DEVROOT)\DLL

#RM	= rm
#MV	= mv
#CP	= cp

RM	= erase
MV	= rename
CP	= copy

CC	= gcc
CXX	= g++

AR	= ar

OPT	= -O3
#OPT	= -O3 -DTEST_ICE
#OPT	= -O2 -DNDEBUG -finline-functions

LFLAGS		= -lwsock32

GC_CFLAGS	= -D__CLEANUP_C
GCE_CFLAGS	= -D__CLEANUP_CXX -x c++ -mthreads

## Mingw32
MAKE		= make
CFLAGS	= $(OPT) -I. -D_WIN32_WINNT=0x400 -DHAVE_CONFIG_H -Wall

## Cygwin G++
#CFLAGS	= $(OPT) -x $(GLANG) -fhandle-exceptions -D_WIN32_WINNT=0x400 -I. -DHAVE_CONFIG_H -Wall

# Agregate modules for inlinability
DLL_OBJS	= \
		attr.o \
		barrier.o \
		cancel.o \
		cleanup.o \
		condvar.o \
		create.o \
		dll.o \
		errno.o \
		exit.o \
		fork.o \
		global.o \
		misc.o \
		mutex.o \
		nonportable.o \
		private.o \
		rwlock.o \
		sched.o \
		semaphore.o \
		signal.o \
		spin.o \
		sync.o \
		tsd.o

# Separate modules for minimum size statically linked images
SMALL_STATIC_OBJS	= \
		attr_is_attr.o \
		attr_init.o \
		attr_destroy.o \
		attr_getdetachstate.o \
		attr_setdetachstate.o \
		attr_getstackaddr.o \
		attr_setstackaddr.o \
		attr_getstacksize.o \
		attr_setstacksize.o \
		attr_getscope.o \
		attr_setscope.o \
		barrier_init.o \
		barrier_destroy.o \
		barrier_wait.o \
		barrier_attr_init.o \
		barrier_attr_destroy.o \
		barrier_attr_setpshared.o \
		barrier_attr_getpshared.o \
		cancel_setcancelstate.o \
		cancel_setcanceltype.o \
		cancel_testcancel.o \
		cancel_cancel.o \
		cleanup.o \
		condvar_attr_destroy.o \
		condvar_attr_getpshared.o \
		condvar_attr_init.o \
		condvar_attr_setpshared.o \
		condvar_check_need_init.o \
		condvar_destroy.o \
		condvar_init.o \
		condvar_signal.o \
		condvar_wait.o \
		create.o \
		dll.o \
		errno.o \
		exit.o \
		fork.o \
		global.o \
		mutex.o \
		np_mutexattr_setkind.o \
		np_mutexattr_getkind.o \
		np_getw32threadhandle.o \
		np_delay.o \
		np_num_processors.o \
		np_win32_attach.o \
		private.o \
		pthread_equal.o \
		pthread_getconcurrency.o \
		pthread_once.o \
		pthread_self.o \
		pthread_setconcurrency.o \
		ptw32_calloc.o \
		ptw32_new.o \
		rwlock.o \
		sched.o \
		semaphore_init.o \
		semaphore_destroy.o \
		semaphore_trywait.o \
		semaphore_timedwait.o \
		semaphore_wait.o \
		semaphore_post.o \
		semaphore_postmultiple.o \
		semaphore_getvalue.o \
		semaphore_increase.o \
		semaphore_decrease.o \
		semaphore_open.o \
		semaphore_close.o \
		semaphore_unlink.o \
		signal.o \
		spin.o \
		sync.o \
		tsd.o \
		w32_CancelableWait.o

INCL	= \
		config.h \
		implement.h \
		semaphore.h \
		pthread.h \
		need_errno.h

ATTR_SRCS	= \
		attr_is_attr.c \
		attr_init.c \
		attr_destroy.c \
		attr_getdetachstate.c \
		attr_setdetachstate.c \
		attr_getstackaddr.c \
		attr_setstackaddr.c \
		attr_getstacksize.c \
		attr_setstacksize.c \
		attr_getscope.c \
		attr_setscope.c

BARRIER_SRCS = \
		barrier_init.c \
		barrier_destroy.c \
		barrier_wait.c \
		barrier_attr_init.c \
		barrier_attr_destroy.c \
		barrier_attr_setpshared.c \
		barrier_attr_getpshared.c

CANCEL_SRCS	= \
		cancel_setcancelstate.c \
		cancel_setcanceltype.c \
		cancel_testcancel.c \
		cancel_cancel.c 

CONDVAR_SRCS	= \
		condvar_attr_destroy.c \
		condvar_attr_getpshared.c \
		condvar_attr_init.c \
		condvar_attr_setpshared.c \
		condvar_check_need_init.c \
		condvar_destroy.c \
		condvar_init.c \
		condvar_signal.c \
		condvar_wait.c

MISC_SRCS	= \
		pthread_equal.c \
		pthread_getconcurrency.c \
		pthread_once.c \
		pthread_self.c \
		pthread_setconcurrency.c \
		ptw32_calloc.c \
		ptw32_new.c \
		w32_CancelableWait.c

NONPORTABLE_SRCS = \
		np_mutexattr_setkind.c \
		np_mutexattr_getkind.c \
		np_getw32threadhandle.c \
		np_delay.c \
		np_num_processors.c \
		np_win32_attach.c

SEMAPHORE_SRCS = \
		semaphore_init.c \
		semaphore_destroy.c \
		semaphore_trywait.c \
		semaphore_timedwait.c \
		semaphore_wait.c \
		semaphore_post.c \
		semaphore_postmultiple.c \
		semaphore_getvalue.c \
		semaphore_increase.c \
		semaphore_decrease.c \
		semaphore_open.c \
		semaphore_close.c \
		semaphore_unlink.c

GC_DLL 	= pthreadGC.dll
GCE_DLL	= pthreadGCE.dll

GC_LIB	= libpthreadGC.a
GCE_LIB	= libpthreadGCE.a


all:
	@ echo Run one of the following command lines:
	@ echo make clean GCE   (to build the GNU C dll with C++ exception handling)
	@ echo make clean GC    (to build the GNU C dll with C cleanup code)

auto:
	@ $(MAKE) clean GCE
	@ $(MAKE) clean GC

GC:
		$(MAKE) CLEANUP_FLAGS="$(GC_CFLAGS)" $(GC_DLL)

GCE:
		$(MAKE) CLEANUP_FLAGS="$(GCE_CFLAGS)" $(GCE_DLL)

tests:
	@ cd tests
	@ $(MAKE) auto

%.pre: %.c
	$(CC) -E -o $@ $(CFLAGS) $^

%.s: %.c
	$(CC) -c $(CFLAGS) -Wa,-ahl $^ > $@

.SUFFIXES: .dll .c .o

.c.o:;		 $(CC) -c -o $@ $(CFLAGS) $(CLEANUP_FLAGS) $<


$(GC_DLL): $(DLL_OBJS)
	$(CC) $(OPT) -shared -o $@ $^ $(LFLAGS)
	dlltool -k --dllname $@ --output-lib $(GC_LIB) --def pthread.def

$(GCE_DLL): $(DLL_OBJS)
	$(CXX) $(OPT) -mthreads -shared -o $@ $^  $(LFLAGS)
	dlltool -k --dllname $@ --output-lib $(GCE_LIB) --def pthread.def

clean:
	-$(RM) *~
	-$(RM) *.pre
	-$(RM) *.o
	-$(RM) *.exe

realclean: clean
	-$(RM) $(GC_LIB)
	-$(RM) $(GCE_LIB)
	-$(RM) $(GC_DLL)
	-$(RM) $(GCE_DLL)

attr.o:		attr.c $(ATTR_SRCS) $(INCL)
barrier.o:	barrier.c $(BARRIER_SRCS) $(INCL)
cancel.o:	cancel.c $(CANCEL_SRCS) $(INCL)
condvar.o:	condvar.c $(CONDVAR_SRCS) $(INCL)
misc.o:		misc.c $(MISC_SRCS) $(INCL)
nonportable.o:	nonportable.c $(NONPORTABLE_SRCS) $(INCL)
semaphore.o:	semaphore.c $(SEMAPHORE_SRCS) $(INCL)

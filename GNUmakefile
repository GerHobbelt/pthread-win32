#
# Pthreads-win32 - POSIX Threads Library for Win32
# Copyright (C) 1998
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
# MA 02111-1307, USA
#

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
#OPT	= -O2 -DNDEBUG -finline-functions

GC_CFLAGS	= -D__CLEANUP_C
GCE_CFLAGS	= -D__CLEANUP_CXX -x c++ -mthreads

## Mingw32
MAKE	= make
CFLAGS	= $(OPT) -I. -D_WIN32_WINNT=0x400 -DHAVE_CONFIG_H -DPTW32_BUILD -Wall

## Cygwin G++
#CFLAGS	= $(OPT) -x $(GLANG) -fhandle-exceptions -D_WIN32_WINNT=0x400 -I. -DHAVE_CONFIG_H -DPTW32_BUILD -Wall

OBJS	= attr.o barrier.o cancel.o cleanup.o condvar.o create.o dll.o errno.o \
	  exit.o fork.o global.o misc.o mutex.o nonportable.o \
	  private.o rwlock.o sched.o semaphore.o signal.o spin.o sync.o tsd.o

INCL	= implement.h semaphore.h pthread.h windows.h

GC_DLL 	= pthreadGC.dll
GCE_DLL = pthreadGCE.dll

GC_LIB	= libpthreadGC.a
GCE_LIB = libpthreadGCE.a


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


$(GC_DLL): $(OBJS)
	$(CC) $(OPT) -shared -o $@ $^
	dlltool -k --dllname $@ --output-lib $(GC_LIB) --def pthread.def

$(GCE_DLL): $(OBJS)
	$(CXX) $(OPT) -mthreads -shared -o $@ $^
	dlltool -k --dllname $@ --output-lib $(GCE_LIB) --def pthread.def

clean:
	-$(RM) *~
	-$(RM) *.o 
	-$(RM) *.exe

realclean: clean
	-$(RM) $(GC_LIB)
	-$(RM) $(GCE_LIB)
	-$(RM) $(GC_DLL)
	-$(RM) $(GCE_DLL)



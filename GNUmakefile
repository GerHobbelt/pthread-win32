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

GLANG	= c++

RM	= erase
MV	= rename
CP	= copy

CC	= gcc

AR	= ar

LD	= gcc -mdll

OPT	= -g -O0 -x $(GLANG)

## Mingw32
CFLAGS	= $(OPT) -I. -mthreads -DHAVE_CONFIG_H -DPTW32_BUILD -Wall

## Cygwin G++
#CFLAGS	= $(OPT) -fhandle-exceptions -I. -DHAVE_CONFIG_H -DPTW32_BUILD -Wall

OBJS	= attr.o cancel.o cleanup.o condvar.o create.o dll.o errno.o \
	  exit.o fork.o global.o misc.o mutex.o nonportable.o \
	  private.o rwlock.o sched.o semaphore.o signal.o sync.o tsd.o

INCL	= implement.h semaphore.h pthread.h windows.h

DLL     = pthreadGCE.dll

LIBS	= libpthreadw32.a


all:	$(LIBS)

fake.a:
	@ $(CP) pthreadVCE.dll $(DLL)
	dlltool --def pthread.def --output-lib $@ --dllname $(DLL)
	@-$(RM) $(LIBS)
	$(MV) fake.a $(LIBS)

$(LIBS): $(DLL)
	dlltool --def pthread.def --output-lib $@ --dllname $(DLL)

%.pre: %.c
	$(CC) -E -o $@ $(CFLAGS) $^

%.s: %.c
	$(CC) -c $(CFLAGS) -Wa,-ahl $^ > $@

.SUFFIXES: .dll

$(DLL): $(OBJS)
	$(LD) -o $@ $^ -Wl,--base-file,$*.base
	dlltool --base-file=$*.base --def pthread.def --output-exp $*.exp --dllname $@
	$(LD) -o $@ $^ -Wl,--base-file,$*.base,$*.exp
	dlltool --base-file=$*.base --def pthread.def --output-exp $*.exp --dllname $@
	$(LD) -o $@ $^ -Wl,$*.exp

clean:
	-$(RM) *~
	-$(RM) $(LIBS)
	-$(RM) *.o 
	-$(RM) *.exe
	-$(RM) $(DLL) 
	-$(RM) $(DLL:.dll=.base)
	-$(RM) $(DLL:.dll=.exp)
	-$(RM) fake.a

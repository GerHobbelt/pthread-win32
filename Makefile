RM	= erase

CC	= g++

AR	= ar

LD	= gcc -mdll -e _DllMain@12

OPT	= -g -O2

## Mingw32
CFLAGS	= $(OPT) -I. -DHAVE_CONFIG_H -Wall

## Cygwin G++
#CFLAGS	= $(OPT) -fhandle-exceptions -I. -DHAVE_CONFIG_H -Wall

OBJS	= attr.o cancel.o cleanup.o condvar.o create.o dll.o \
	  exit.o fork.o global.o misc.o mutex.o private.o sched.o \
	  semaphore.o signal.o sync.o tsd.o

INCL	= implement.h pthread.h windows.h

DLL     = pthread.dll

LIB	= libpthread32.a


all:	$(LIB)

$(LIB): $(DLL)
	dlltool --def $(DLL:.dll=.def) --output-lib $@ --dllname $(DLL)

.SUFFIXES: .dll

$(DLL): $(OBJS)
	$(LD) -o $@ $^ -Wl,--base-file,$*.base
	dlltool --base-file=$*.base --def $*.def --output-exp $*.exp --dllname $@
	$(LD) -o $@ $^ -Wl,--base-file,$*.base,$*.exp
	dlltool --base-file=$*.base --def $*.def --output-exp $*.exp --dllname $@
	$(LD) -o $@ $^ -Wl,$*.exp

clean:
	-$(RM) *~
	-$(RM) $(LIB)
	-$(RM) *.o 
	-$(RM) *.exe
	-$(RM) $(DLL) 
	-$(RM) $(DLL:.dll=.base)
	-$(RM) $(DLL:.dll=.exp)

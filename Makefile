
CFLAGS	= -I. -DHAVE_CONFIG_H

SRCS	= attr.c cancel.c cleanup.c condvar.c create.c dll.c \
	  exit.c fork.c global.c misc.c mutex.c private.c sched.c \
	  signal.c sync.c tsd.c

OBJS	= attr.o cancel.o cleanup.o condvar.o create.o dll.o \
	  exit.o fork.o global.o misc.o mutex.o private.o sched.o \
	  signal.o sync.o tsd.o

INCL	= implement.h pthread.h windows.h

all:	$(OBJS)
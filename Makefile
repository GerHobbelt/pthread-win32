
CFLAGS	= -I. -DHAVE_CONFIG_H -Wall

OBJS	= attr.o cancel.o cleanup.o condvar.o create.o dll.o \
	  exit.o fork.o global.o misc.o mutex.o private.o sched.o \
	  signal.o sync.o tsd.o windows.o

INCL	= implement.h pthread.h windows.h

LIB	= libpthread32.a

all:	$(LIB)

$(LIB):	$(OBJS)
	$(AR) r $(LIB) $(OBJS)

clean:
	-rm $(LIB) *.o

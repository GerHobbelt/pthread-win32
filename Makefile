
# This makefile is compatible with MS nmake and can be used as a
# replacement for buildlib.bat. I've changed the target from an ordinary dll
# (/LD) to a debugging dll (/LDd).
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

DEVROOT=e:

DLLDEST=$(DEVROOT)\dll
LIBDEST=$(DEVROOT)\lib

#CLIB	= /MD
CLIB	= /MT
CFLAGS	= #/EHsc /TP

OBJ=attr.obj \
	cancel.obj \
	cleanup.obj \
	condvar.obj \
	create.obj \
	dll.obj \
	errno.obj \
	exit.obj \
	fork.obj \
	global.obj \
	misc.obj \
	mutex.obj \
	private.obj \
	rwlock.obj \
	sched.obj \
	semaphore.obj \
	signal.obj \
	sync.obj \
	tsd.obj

all: pthread.dll

clean:
	del pthread.dll \
		pthread.lib \
		*.obj


install: all
	copy pthread.dll $(DLLDEST)
	copy pthread.lib $(LIBDEST)

pthread.dll: $(OBJ) pthread.def
	cl /LD /Zi $(CFLAGS) $(OBJ) /Fepthread.dll /link \
	    /nodefaultlib:libcmt \
		/implib:pthread.lib \
		msvcrt.lib \
		/def:pthread.def

.c.obj::
	cl /W3 $(CLIB) $(CFLAGS) /nologo /Yd /Zi /I. \
		/D_WIN32_WINNT=0x400 \
		/DSTDCALL=_stdcall \
		-c $<

$(OBJ):


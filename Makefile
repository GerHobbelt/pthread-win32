
# This makefile is compatible with MS nmake and can be used as a
# replacement for buildlib.bat. I've changed the target from an ordinary dll
# (/LD) to a debugging dll (/LDd).
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

DEVROOT=c:\pthreads\dll

DLLDEST=$(DEVROOT)
LIBDEST=$(DEVROOT)

DLLS	= pthreadVCE.dll pthreadVSE.dll

# C++ Exceptions
VCEFLAGS	= /GX /TP /DPtW32NoCatchWarn
#Structured Exceptions
VSEFLAGS	= 

CFLAGS	= /W3 /MT /nologo /Yd /Zi /I. /D_WIN32_WINNT=0x400

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

all:
	@ echo Run one of the following command lines:
	@ echo nmake clean VCE   (to build the dll with C++ exception handling)
	@ echo nmake clean VSE   (to build the dll with structured exception handling)

VCE:
	@ nmake /nologo EHFLAGS="$(VCEFLAGS)" pthreadVCE.dll

VSE:
	@ nmake /nologo EHFLAGS="$(VSEFLAGS)" pthreadVSE.dll

realclean: clean
	del *.dll
	del *.lib

clean:
	del *.obj
	del *.ilk
	del *.pdb
	del *.exp
	del *.o


install: $(DLLS)
	copy pthread*.dll $(DLLDEST)
	copy pthread*.lib $(LIBDEST)

$(DLLS): $(OBJ) pthread.def
	cl /LD /Zi /nologo $(OBJ) \
		/link /nodefaultlib:libcmt /implib:$*.lib \
		msvcrt.lib /def:pthread.def /out:$@

.c.obj:
	cl $(EHFLAGS) $(CFLAGS) -c $<


# This makefile is compatible with MS nmake and can be used as a
# replacement for buildlib.bat. I've changed the target from an ordinary dll
# (/LD) to a debugging dll (/LDd).
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

DEVROOT=c:\pthreads\dll

DLLDEST=$(DEVROOT)
LIBDEST=$(DEVROOT)

DLLS	= pthreadVCE.dll pthreadVSE.dll pthreadVC.dll

# C++ Exceptions
VCEFLAGS	= /GX /TP /DPtW32NoCatchWarn /D__CLEANUP_CXX
#Structured Exceptions
VSEFLAGS	= /D__CLEANUP_SEH
#C cleanup code
VCFLAGS	= /D__CLEANUP_C

CFLAGS	= /W3 /MT /nologo /Yd /Zi /I. /D_WIN32_WINNT=0x400 /DPTW32_BUILD

OBJ=  attr.obj \
	barrier.obj \
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
	nonportable.obj \
	private.obj \
	rwlock.obj \
	sched.obj \
	semaphore.obj \
	signal.obj \
	spin.obj \
	sync.obj \
	tsd.obj

all:
	@ echo Run one of the following command lines:
	@ echo nmake clean VCE   (to build the MSVC dll with C++ exception handling)
	@ echo nmake clean VSE   (to build the MSVC dll with structured exception handling)
	@ echo nmake clean VC    (to build the MSVC dll with C cleanup code)

auto:
	@ nmake clean VCE
	@ nmake clean VSE
	@ nmake clean VC

VCE:
	@ nmake /nologo EHFLAGS="$(VCEFLAGS)" pthreadVCE.dll

VSE:
	@ nmake /nologo EHFLAGS="$(VSEFLAGS)" pthreadVSE.dll

VC:
	@ nmake /nologo EHFLAGS="$(VCFLAGS)" pthreadVC.dll

realclean: clean
	if exist *.dll del *.dll
	if exist *.lib del *.lib

clean:
	if exist *.obj del *.obj
	if exist *.ilk del *.ilk
	if exist *.pdb del *.pdb
	if exist *.exp del *.exp
	if exist *.o del *.o


install: $(DLLS)
	copy pthread*.dll $(DLLDEST)
	copy pthread*.lib $(LIBDEST)

$(DLLS): $(OBJ) pthread.def
	cl /LD /Zi /nologo $(OBJ) \
		/link /nodefaultlib:libcmt /implib:$*.lib \
		msvcrt.lib /def:pthread.def /out:$@

.c.obj:
	cl $(EHFLAGS) $(CFLAGS) -c $<

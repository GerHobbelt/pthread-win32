# This makefile is compatible with MS nmake and can be used as a
# replacement for buildlib.bat. I've changed the target from an ordinary dll
# (/LD) to a debugging dll (/LDd).
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.

DEVROOT=c:\pthreads

DLLDEST=$(DEVROOT)\DLL
LIBDEST=$(DEVROOT)\DLL

DLLS	= pthreadVCE.dll pthreadVSE.dll pthreadVC.dll

OPTIM	= /O2

# C++ Exceptions
VCEFLAGS	= /GX /TP /D__CLEANUP_CXX
#Structured Exceptions
VSEFLAGS	= /D__CLEANUP_SEH
#C cleanup code
VCFLAGS	= /D__CLEANUP_C

#CFLAGS	= $(OPTIM) /W3 /MT /nologo /Yd /Zi /I. /D_WIN32_WINNT=0x400 /DHAVE_CONFIG_H /DTEST_ICE
CFLAGS	= $(OPTIM) /W3 /MT /nologo /Yd /Zi /I. /D_WIN32_WINNT=0x400 /DHAVE_CONFIG_H

# Agregate modules for inlinability
DLL_OBJS	= \
		attr.obj \
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

# Separate modules for minimising the size of statically linked images
SMALL_STATIC_OBJS	= \
		attr_is_attr.obj \
		attr_init.obj \
		attr_destroy.obj \
		attr_getdetachstate.obj \
		attr_setdetachstate.obj \
		attr_getstackaddr.obj \
		attr_setstackaddr.obj \
		attr_getstacksize.obj \
		attr_setstacksize.obj \
		attr_getscope.obj \
		attr_setscope.obj \
		barrier_init.obj \
		barrier_destroy.obj \
		barrier_wait.obj \
		barrier_attr_init.obj \
		barrier_attr_destroy.obj \
		barrier_attr_setpshared.obj \
		barrier_attr_getpshared.obj \
		cancel_setcancelstate.obj \
		cancel_setcanceltype.obj \
		cancel_testcancel.obj \
		cancel_cancel.obj \
		cleanup.obj \
		condvar_attr_destroy.obj \
		condvar_attr_getpshared.obj \
		condvar_attr_init.obj \
		condvar_attr_setpshared.obj \
		condvar_check_need_init.obj \
		condvar_destroy.obj \
		condvar_init.obj \
		condvar_signal.obj \
		condvar_wait.obj \
		create.obj \
		dll.obj \
		errno.obj \
		exit.obj \
		fork.obj \
		global.obj \
		mutex.obj \
		np_mutexattr_setkind.obj \
		np_mutexattr_getkind.obj \
		np_getw32threadhandle.obj \
		np_delay.obj \
		np_num_processors.obj \
		np_win32_attach.obj \
		private.obj \
		pthread_equal.obj \
		pthread_getconcurrency.obj \
		pthread_once.obj \
		pthread_self.obj \
		pthread_setconcurrency.obj \
		ptw32_calloc.obj \
		ptw32_new.obj \
		rwlock.obj \
		sched.obj \
		semaphore_init.obj \
		semaphore_destroy.obj \
		semaphore_trywait.obj \
		semaphore_timedwait.obj \
		semaphore_wait.obj \
		semaphore_post.obj \
		semaphore_postmultiple.obj \
		semaphore_getvalue.obj \
		semaphore_increase.obj \
		semaphore_decrease.obj \
		semaphore_open.obj \
		semaphore_close.obj \
		semaphore_unlink.obj \
		signal.obj \
		spin.obj \
		sync.obj \
		tsd.obj \
		w32_CancelableWait.obj

INCL	= config.h implement.h semaphore.h pthread.h need_errno.h

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

$(DLLS): $(DLL_OBJS) pthread.def
	cl /LD /Zi /nologo $(DLL_OBJS) \
		/link /nodefaultlib:libcmt /implib:$*.lib \
		msvcrt.lib wsock32.lib /def:pthread.def /out:$@

.c.obj:
	cl $(EHFLAGS) $(CFLAGS) -c $<

attr.obj:	attr.c $(ATTR_SRCS) $(INCL)
barrier.obj:	barrier.c $(BARRIER_SRCS) $(INCL)
cancel.obj:	cancel.c $(CANCEL_SRCS) $(INCL)
condvar.obj:	condvar.c $(CONDVAR_SRCS) $(INCL)
misc.obj:	misc.c $(MISC_SRCS) $(INCL)
nonportable.obj:	nonportable.c $(NONPORTABLE_SRCS) $(INCL)
semaphore.obj:	semaphore.c $(SEMAPHORE_SRCS) $(INCL)


# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.
#
# --------------------------------------------------------------------------
#
#      Pthreads-win32 - POSIX Threads Library for Win32
#      Copyright(C) 1998 John E. Bossom
#      Copyright(C) 1999,2012 Pthreads-win32 contributors
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

DLL_VER	= 2$(EXTRAVERSION)

CP	= copy
RM	= erase
RMDIR	= rmdir /s /q
CAT	= type
MKDIR	= mkdir
ECHO	= echo
TOUCH	= $(ECHO) touched >

# The next path is relative to $BUILD_DIR
QAPC	= # ..\QueueUserAPCEx\User\quserex.dll

CPHDR	= pthread.h semaphore.h sched.h

OPTIM	= /O2 /Ob0

XXLIBS	= ws2_32.lib

# C++ Exceptions
VCEFLAGS	= /EHs /TP /DPtW32NoCatchWarn /D__CLEANUP_CXX
VCELIB	= pthreadVCE$(DLL_VER).lib
VCEDLL	= pthreadVCE$(DLL_VER).dll
VCELIBD	= pthreadVCE$(DLL_VER)d.lib
VCEDLLD	= pthreadVCE$(DLL_VER)d.dll
# Structured Exceptions
VSEFLAGS	= /D__CLEANUP_SEH
VSELIB	= pthreadVSE$(DLL_VER).lib
VSEDLL	= pthreadVSE$(DLL_VER).dll
VSELIBD	= pthreadVSE$(DLL_VER)d.lib
VSEDLLD	= pthreadVSE$(DLL_VER)d.dll
# C cleanup code
VCFLAGS	= /D__CLEANUP_C
VCLIB	= pthreadVC$(DLL_VER).lib
VCDLL	= pthreadVC$(DLL_VER).dll
VCLIBD	= pthreadVC$(DLL_VER)d.lib
VCDLLD	= pthreadVC$(DLL_VER)d.dll
# C++ Exceptions in application - using VC version of pthreads dll
VCXFLAGS	= /EHs /TP /D__CLEANUP_C

# Defaults
CPLIB	= $(VCLIB)
CPDLL	= $(VCDLL)

CFLAGS= $(OPTIM) /W3 /MD /nologo /Z7
LFLAGS= /INCREMENTAL:NO
INCLUDES=-I.
BUILD_DIR=..

EHFLAGS		=

# If a test case returns a non-zero exit code to the shell, make will
# stop.

include common.mk

#
# Define "FAST=foo bar" to build and run only the tests "foo.exe" and "bar.exe"
# without any prerequisite tests. Set FAST to one or more tests.
#
# To build and run "foo.exe" and "bar.exe" and run all prerequisite tests
# use "TESTS=foo bar". Set TESTS to one or more tests.
#
# You can't use FAST with TESTS.
#
!IFDEF FAST
TESTS	= $(FAST)
!ELSE
include runorder.mk
!ENDIF

help:
	@ $(ECHO) Run one of the following command lines:
	@ $(ECHO) nmake clean VC
	@ $(ECHO) nmake clean VC-bench
	@ $(ECHO) nmake clean VC-static
	@ $(ECHO) nmake clean VC-static-bench
	@ $(ECHO) nmake clean VC-debug
	@ $(ECHO) nmake clean VC-static-debug
	@ $(ECHO) nmake clean VC-small-static-debug
	@ $(ECHO) nmake clean VCX
	@ $(ECHO) nmake clean VCX-bench
	@ $(ECHO) nmake clean VCX-static
	@ $(ECHO) nmake clean VCX-static-bench
	@ $(ECHO) nmake clean VCX-debug
	@ $(ECHO) nmake clean VCX-static-debug
	@ $(ECHO) nmake clean VCX-small-static-debug
	@ $(ECHO) nmake clean VCE
	@ $(ECHO) nmake clean VCE-bench
	@ $(ECHO) nmake clean VCE-static
	@ $(ECHO) nmake clean VCE-static-bench
	@ $(ECHO) nmake clean VCE-debug
	@ $(ECHO) nmake clean VCE-static-debug
	@ $(ECHO) nmake clean VCE-small-static-debug
	@ $(ECHO) nmake clean VSE
	@ $(ECHO) nmake clean VSE-bench
	@ $(ECHO) nmake clean VSE-static
	@ $(ECHO) nmake clean VSE-static-bench
	@ $(ECHO) nmake clean VSE-debug
	@ $(ECHO) nmake clean VSE-static-debug
	@ $(ECHO) nmake clean VSE-small-static-debug

VC:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCFLAGS)" runtests

VCE:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIB)" CPDLL="$(VCEDLL)" EHFLAGS="$(VCEFLAGS)" runtests

VSE:	
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIB)" CPDLL="$(VSEDLL)" EHFLAGS="$(VSEFLAGS)" runtests

VCX:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCXFLAGS)" runtests

VC-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCFLAGS)" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VCE-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIB)" CPDLL="$(VCEDLL)" EHFLAGS="$(VCEFLAGS)" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VSE-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIB)" CPDLL="$(VSEDLL)" EHFLAGS="$(VSEFLAGS)" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VCX-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCXFLAGS)" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VC-static VC-small-static:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="" EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB" runtests

VCE-static VCE-small-static:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIB)" CPDLL="" EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB" runtests

VSE-static VSE-small-static:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIB)" CPDLL="" EHFLAGS="$(VSEFLAGS) /DPTW32_STATIC_LIB" runtests

VCX-static VCX-small-static:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="" EHFLAGS="$(VCXFLAGS) /DPTW32_STATIC_LIB" runtests

VC-static-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="" EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VCE-static-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIB)" CPDLL="" EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VSE-static-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIB)" CPDLL="" EHFLAGS="$(VSEFLAGS) /DPTW32_STATIC_LIB" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VCX-static-bench:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIB)" CPDLL="" EHFLAGS="$(VCXFLAGS) /DPTW32_STATIC_LIB" XXLIBS="benchlib.obj $(XXLIBS)" $(BENCHTESTS)

VC-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIBD)" CPDLL="$(VCDLLD)" EHFLAGS="$(VCFLAGS)" runtests

VC-static-debug VC-small-static-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIBD)" CPDLL="" EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB" runtests

VCE-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIBD)" CPDLL="$(VCEDLLD)" EHFLAGS="$(VCEFLAGS)" runtests

VCE-static-debug VCE-small-static-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCELIBD)" CPDLL="" EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB" runtests

VSE-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIBD)" CPDLL="$(VSEDLLD)" EHFLAGS="$(VSEFLAGS)" runtests

VSE-static-debug VSE-small-static-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VSELIBD)" CPDLL="" EHFLAGS="$(VSEFLAGS) /DPTW32_STATIC_LIB" runtests

VCX-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIBD)" CPDLL="$(VCDLLD)" EHFLAGS="$(VCXFLAGS)" runtests

VCX-static-debug VCX-small-static-debug:
	@ -setenv
	@ $(MAKE) /E /nologo TEST="$@" CPLIB="$(VCLIBD)" CPDLL="" EHFLAGS="$(VCXFLAGS) /DPTW32_STATIC_LIB" runtests

clean:
	if exist passes.mk $(RM) passes.mk
	if exist allpasses.mk $(RM) allpasses.mk
	if exist *.dll $(RM) *.dll
	if exist *.lib $(RM) *.lib
	if exist pthread.h $(RM) pthread.h
	if exist semaphore.h $(RM) semaphore.h
	if exist sched.h $(RM) sched.h
	if exist *.e $(RM) *.e
	if exist *.i $(RM) *.i
	if exist *.obj $(RM) *.obj
	if exist *.pdb $(RM) *.pdb
	if exist *.o $(RM) *.o
	if exist *.asm $(RM) *.asm
	if exist *.exe $(RM) *.exe
	if exist *.manifest $(RM) *.manifest
	if exist *.pass $(RM) *.pass
	if exist *.bench $(RM) *.bench
	if exist *.log $(RM) *.log
	if exist SIZES.V* $(RM) SIZES.V*

# If someone can tell me how to do the equivalent of GNU make $(TESTS:%=%.pass)
# then we can get rid of these two rules and the associated includes below.
# PS. I know about $(TESTS:.ext=.newext) but that requires already having a
# filename extension.
allpasses.mk: $(ALL_KNOWN_TESTS)
	@ $(ECHO) ALL_KNOWN_PASSES = \> allpasses.mk
	@ !if exist $?.c $(ECHO) $?.pass \>> allpasses.mk

passes.mk: $(TESTS)
	@ $(ECHO) PASSES = \> passes.mk
	@ !if exist $?.c $(ECHO) $?.pass \>> passes.mk

runtests: allpasses.mk passes.mk
	@ $(ECHO) Running tests
	@ $(MAKE) /E /nologo PTW32_INCLUDE_PASSES=YES allpassed

!IFDEF PTW32_INCLUDE_PASSES
include allpasses.mk
include passes.mk
# For some reason the dependency below doesn't trigger the inference rule .c.exe below
# for certain tests, while others do. So the compile command is included in
# the command list below and the dependency is commented out, which works.
$(ALL_KNOWN_PASSES): #$(@R).exe
	$(CC) /DPTW32_DLL_PATH="\"$(CPDLL)\"" $(EHFLAGS) $(CFLAGS) $(INCLUDES) $*.c /Fe$*.exe /link $(LFLAGS) $(CPLIB) $(XXLIBS)
	@ $(ECHO) ... Running $(TEST) test: $*.exe
	@ .\$*.exe
	@ $(ECHO) ...... Passed
	@ $(TOUCH) $*.pass

allpassed: $(PASSES)
!ENDIF

$(ALL_KNOWN_TESTS): $(CPHDR) $(CPLIB) $(CPDLL) $(QAPC)

allpassed:
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

$(BENCHTESTS): $(CPHDR) $(CPLIB) $(CPDLL) $(QAPC) benchlib.obj $@.exe
	@ $(ECHO) ... Running $(TEST) benchmark: $*.exe
	@ .\$*.exe
	@ $(ECHO) ...... Done

#.c.exe:
#	$(CC) $(EHFLAGS) $(CFLAGS) $(INCLUDES) $< /Fe$@ /link $(LFLAGS) $(CPLIB) $(XXLIBS)

.c.obj:
	$(CC) $(EHFLAGS) $(CFLAGS) $(INCLUDES) /c $< /Fo$@

.c.i:
	$(CC) /P $(EHFLAGS) $(CFLAGS) $(INCLUDES) $<

$(CPHDR) $(CPLIB) $(CPDLL) $(QAPC):
	@ if exist $(BUILD_DIR)\$@ $(ECHO) Copying $(BUILD_DIR)\$@ && $(CP) $(BUILD_DIR)\$@ .

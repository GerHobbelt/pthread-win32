# This makefile is compatible with MS nmake
# 
# The variables $DLLDEST and $LIBDEST hold the destination directories for the
# dll and the lib, respectively. Probably all that needs to change is $DEVROOT.


# DLL_VER:
# See pthread.h and README - This number is computed as 'current - age'
DLL_VER	= 2
DLL_VERD= $(DLL_VER)d

DEVROOT	= C:\pthreads

DLLDEST	= $(DEVROOT)\dll
LIBDEST	= $(DEVROOT)\lib
HDRDEST	= $(DEVROOT)\include

DLLS	= pthreadVCE$(DLL_VER).dll pthreadVSE$(DLL_VER).dll pthreadVC$(DLL_VER).dll \
		  pthreadVCE$(DLL_VERD).dll pthreadVSE$(DLL_VERD).dll pthreadVC$(DLL_VERD).dll
INLINED_STAMPS	= pthreadVCE$(DLL_VER).stamp pthreadVSE$(DLL_VER).stamp pthreadVC$(DLL_VER).stamp \
				  pthreadVCE$(DLL_VERD).stamp pthreadVSE$(DLL_VERD).stamp pthreadVC$(DLL_VERD).stamp
STATIC_STAMPS	= pthreadVCE$(DLL_VER).static pthreadVSE$(DLL_VER).static pthreadVC$(DLL_VER).static \
				  pthreadVCE$(DLL_VERD).static pthreadVSE$(DLL_VERD).static pthreadVC$(DLL_VERD).static
SMALL_STATIC_STAMPS	= pthreadVCE$(DLL_VER).small_stamp pthreadVSE$(DLL_VER).small_stamp pthreadVC$(DLL_VER).small_stamp \
				  pthreadVCE$(DLL_VERD).small_stamp pthreadVSE$(DLL_VERD).small_stamp pthreadVC$(DLL_VERD).small_stamp

CC	= cl
CPPFLAGS = /I. /DHAVE_PTW32_CONFIG_H
XCFLAGS = /W3 /MD /nologo
CFLAGS	= /O2 /Ob2 $(XCFLAGS)
CFLAGSD	= /Z7 $(XCFLAGS)

# Uncomment this if config.h defines RETAIN_WSALASTERROR
#XLIBS = wsock32.lib

# Default cleanup style
CLEANUP	= __CLEANUP_C

# C++ Exceptions
VCEFLAGS	= /EHs /TP $(CPPFLAGS) $(CFLAGS)
VCEFLAGSD	= /EHs /TP $(CPPFLAGS) $(CFLAGSD)
#Structured Exceptions
VSEFLAGS	= $(CPPFLAGS) $(CFLAGS)
VSEFLAGSD	= $(CPPFLAGS) $(CFLAGSD)
#C cleanup code
VCFLAGS		= $(CPPFLAGS) $(CFLAGS)
VCFLAGSD	= $(CPPFLAGS) $(CFLAGSD)

OBJEXT = obj
RESEXT = res
 
include common.mk

DLL_INLINED_OBJS	= $(DLL_INLINED_OBJS) $(RESOURCE_OBJS)
DLL_OBJS	= $(DLL_OBJS) $(RESOURCE_OBJS)
SMALL_STATIC_OBJS	= $(SMALL_STATIC_OBJS) $(RESOURCE_OBJS)

help:
	@ echo Run one of the following command lines:
	@ echo nmake clean VC
	@ echo nmake clean VC-inlined
	@ echo nmake clean VC-static
	@ echo nmake clean VC-small-static
	@ echo nmake clean VC-debug
	@ echo nmake clean VC-inlined-debug
	@ echo nmake clean VC-static-debug
	@ echo nmake clean VC-small-static-debug
	@ echo nmake clean VCE
	@ echo nmake clean VCE-inlined
	@ echo nmake clean VCE-debug
	@ echo nmake clean VCE-inlined-debug
	@ echo nmake clean VCE-static-debug
	@ echo nmake clean VCE-small-static-debug
	@ echo nmake clean VSE
	@ echo nmake clean VSE-inlined
	@ echo nmake clean VSE-debug
	@ echo nmake clean VSE-inlined-debug

all:
	$(MAKE) /E clean VCE-inlined
	$(MAKE) /E clean VSE-inlined
	$(MAKE) /E clean VC-inlined
	$(MAKE) /E clean VCE-inlined-debug
	$(MAKE) /E clean VSE-inlined-debug
	$(MAKE) /E clean VC-inlined-debug

TEST_ENV = CFLAGS="$(CFLAGS) /DNO_ERROR_DIALOGS"

all-tests:
	$(MAKE) /E realclean VC
	cd tests && $(MAKE) /E clean VC $(TEST_ENV) && $(MAKE) /E clean VCX $(TEST_ENV)
	$(MAKE) /E realclean VCE
	cd tests && $(MAKE) /E clean VCE $(TEST_ENV)
	$(MAKE) /E realclean VSE
	cd tests && $(MAKE) /E clean VSE $(TEST_ENV)
	$(MAKE) /E realclean VC-inlined
	cd tests && $(MAKE) /E clean VC $(TEST_ENV) && $(MAKE) /E clean VCX $(TEST_ENV)
	$(MAKE) /E realclean VCE-inlined
	cd tests && $(MAKE) /E clean VCE $(TEST_ENV)
	$(MAKE) /E realclean VSE-inlined
	cd tests && $(MAKE) /E clean VSE $(TEST_ENV)
	$(MAKE) /E realclean VC-static
	cd tests && $(MAKE) /E clean VC-static $(TEST_ENV)
	$(MAKE) /E realclean VC-small-static
	cd tests && $(MAKE) /E clean VC-static $(TEST_ENV)
	@ echo $@ completed successfully.

all-tests-cflags:
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MD /nologo"
	$(MAKE) all-tests XCFLAGS="/W3 /WX /MT /nologo"
	@ echo $@ completed successfully.

VCE:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS)" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).dll

VCE-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD)" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).dll

VSE:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGS)" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VER).dll

VSE-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGSD)" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VERD).dll

VC:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS)" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).dll

VC-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD)" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).dll

#
# The so-called inlined DLL is just a single translation unit with
# inlining optimisation turned on.
#
VCE-inlined:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).stamp

VCE-inlined-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).stamp

VSE-inlined:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VER).stamp

VSE-inlined-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VSEFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_SEH pthreadVSE$(DLL_VERD).stamp

VC-inlined:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).stamp

VC-inlined-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_BUILD_INLINED" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).stamp

#
# Static builds
#

VC-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_BUILD_INLINED /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).static

VC-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_BUILD_INLINED /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).static

VC-small-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGS) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VER).small_stamp

VC-small-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCFLAGSD) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_C pthreadVC$(DLL_VERD).small_stamp

VCE-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_BUILD_INLINED /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).static

VCE-small-static:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGS) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VER).small_stamp

VCE-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_BUILD_INLINED /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).static

VCE-small-static-debug:
	@ $(MAKE) /E /nologo EHFLAGS="$(VCEFLAGSD) /DPTW32_STATIC_LIB" CLEANUP=__CLEANUP_CXX pthreadVCE$(DLL_VERD).small_stamp


realclean: clean
	if exist pthread*.dll del pthread*.dll
	if exist pthread*.lib del pthread*.lib
	if exist *.manifest del *.manifest
	if exist *.stamp del *.stamp
	if exist *.small_stamp del *.small_stamp
	cd tests && $(MAKE) clean

clean:
	if exist *.obj del *.obj
	if exist *.def del *.def
	if exist *.ilk del *.ilk
	if exist *.pdb del *.pdb
	if exist *.exp del *.exp
	if exist *.map del *.map
	if exist *.o del *.o
	if exist *.i del *.i
	if exist *.res del *.res


install:
	if exist pthread*.dll copy pthread*.dll $(DLLDEST)
	copy pthread*.lib $(LIBDEST)
	copy pthread.h $(HDRDEST)
	copy sched.h $(HDRDEST)
	copy semaphore.h $(HDRDEST)

$(DLLS): $(DLL_OBJS)
	$(CC) /LDd /Zi /nologo $(DLL_OBJS) /link /implib:$*.lib $(XLIBS) /out:$@

$(INLINED_STAMPS): $(DLL_INLINED_OBJS)
	$(CC) /LDd /Zi /nologo $(DLL_INLINED_OBJS) /link /implib:$*.lib $(XLIBS) /out:$*.dll

$(STATIC_STAMPS): $(DLL_INLINED_OBJS)
	if exist $*.lib del $*.lib
	lib $(DLL_INLINED_OBJS) /out:$*.lib

$(SMALL_STATIC_STAMPS): $(SMALL_STATIC_OBJS)
	if exist $*.lib del $*.lib
	lib $(SMALL_STATIC_OBJS) /out:$*.lib

.c.obj:
	$(CC) $(EHFLAGS) /D$(CLEANUP) -c $<

# TARGET_CPU is an environment variable set by Visual Studio Command Prompt
# as provided by the SDK
.rc.res:
	rc /dPTW32_ARCH$(TARGET_CPU) /dPTW32_RC_MSC /d$(CLEANUP) $<

.c.i:
	$(CC) /P /O2 /Ob1 $(VCFLAGS) $<

#
# --------------------------------------------------------------------------
#
#      Pthreads-win32 - POSIX Threads Library for Win32
#      Copyright(C) 1998 John E. Bossom
#      Copyright(C) 1999,2005 Pthreads-win32 contributors
# 
#      Contact Email: rpj@callisto.canberra.edu.au
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

DLL_VER	= 2
DLL_VERD= $(DLL_VER)d

DEVROOT	= C:\PTHREADS

DLLDEST	= $(DEVROOT)\DLL
LIBDEST	= $(DEVROOT)\DLL

# If Running MsysDTK
RM	= rm -f
MV	= mv -f
CP	= cp -f
ECHO	= echo

# If not.
#RM	= erase
#MV	= rename
#CP	= copy
#ECHO	= echo

# For cross compiling use e.g.
# make CROSS=x86_64-w64-mingw32- clean GC-inlined
CROSS	= 

AR	= $(CROSS)ar
DLLTOOL = $(CROSS)dlltool
CC      = $(CROSS)gcc
CXX     = $(CROSS)g++
RANLIB  = $(CROSS)ranlib
RC	= $(CROSS)windres

OPT	= $(CLEANUP) -O3 # -finline-functions -findirect-inlining
XOPT	=

RCFLAGS		= --include-dir=.
LFLAGS		=
# Uncomment this if config.h defines RETAIN_WSALASTERROR
#LFLAGS		+= -lws2_32
# Uncomment this to link the GCC/C++ runtime libraries statically
# (Note: Be sure to read about these options and their associated caveats
# at http://gcc.gnu.org/onlinedocs/gcc/Link-Options.html)
# PLEASE NOTE: If you do this DO NOT distribute your pthreads DLLs with
# the official filenaming, i.e. pthreadVC2.dll, etc. Instead, change DLL_VER
# above to "2slgcc" for example, to build "pthreadGC2slgcc.dll", etc.
#LFLAGS		+= -static-libgcc -static-libstdc++

# ----------------------------------------------------------------------
# The library can be built with some alternative behaviour to
# facilitate development of applications on Win32 that will be ported
# to other POSIX systems. Nothing definable here will make the library
# non-compliant, but applications that make assumptions that POSIX
# does not garrantee may fail or misbehave under some settings.
#
# PTW32_THREAD_ID_REUSE_INCREMENT
# Purpose:
# POSIX says that applications should assume that thread IDs can be
# recycled. However, Solaris and some other systems use a [very large]
# sequence number as the thread ID, which provides virtual uniqueness.
# Pthreads-win32 provides pseudo-unique IDs when the default increment
# (1) is used, but pthread_t is not a scalar type like Solaris's.
#
# Usage:
# Set to any value in the range: 0 <= value <= 2^wordsize
#
# Examples:
# Set to 0 to emulate non recycle-unique behaviour like Linux or *BSD.
# Set to 1 for recycle-unique thread IDs (this is the default).
# Set to some other +ve value to emulate smaller word size types
# (i.e. will wrap sooner).
#
#PTW32_FLAGS	= "-DPTW32_THREAD_ID_REUSE_INCREMENT=0"
#
# ----------------------------------------------------------------------

GC_CFLAGS	= $(PTW32_FLAGS) 
GCE_CFLAGS	= $(PTW32_FLAGS) -mthreads

## Mingw
#MAKE		?= make
CFLAGS	= $(OPT) $(XOPT) -I. -DHAVE_PTW32_CONFIG_H -Wall

OBJEXT = o
RESEXT = o
 
include common.mk

DLL_INLINED_OBJS += $(RESOURCE_OBJS)
DLL_OBJS += $(RESOURCE_OBJS)
SMALL_STATIC_OBJS += $(RESOURCE_OBJS)

GCE_DLL	= pthreadGCE$(DLL_VER).dll
GCED_DLL= pthreadGCE$(DLL_VERD).dll
GCE_LIB	= libpthreadGCE$(DLL_VER).a
GCED_LIB= libpthreadGCE$(DLL_VERD).a
GCE_INLINED_STAMP = pthreadGCE$(DLL_VER).stamp
GCED_INLINED_STAMP = pthreadGCE$(DLL_VERD).stamp

GC_DLL 	= pthreadGC$(DLL_VER).dll
GCD_DLL	= pthreadGC$(DLL_VERD).dll
GC_LIB	= libpthreadGC$(DLL_VER).a
GCD_LIB	= libpthreadGC$(DLL_VERD).a
GC_INLINED_STAMP = pthreadGC$(DLL_VER).stamp
GCD_INLINED_STAMP = pthreadGC$(DLL_VERD).stamp
GC_STATIC_STAMP = libpthreadGC$(DLL_VER).stamp
GCD_STATIC_STAMP = libpthreadGC$(DLL_VERD).stamp
GC_SMALL_STATIC_STAMP = libpthreadGC$(DLL_VER).small_stamp
GCD_SMALL_STATIC_STAMP = libpthreadGC$(DLL_VERD).small_stamp

PTHREAD_DEF	= pthread.def

help:
	@ echo "Run one of the following command lines:"
	@ echo "$(MAKE) clean GC                    (to build the GNU C dll with C cleanup code)"
	@ echo "$(MAKE) clean GCE                   (to build the GNU C dll with C++ exception handling)"
	@ echo "$(MAKE) clean GC-inlined            (to build the GNU C inlined dll with C cleanup code)"
	@ echo "$(MAKE) clean GCE-inlined           (to build the GNU C inlined dll with C++ exception handling)"
	@ echo "$(MAKE) clean GC-static             (to build the GNU C inlined static lib with C cleanup code)"
	@ echo "$(MAKE) clean GC-small-static       (to build the GNU C small static lib with C cleanup code)"
	@ echo "$(MAKE) clean GC-debug              (to build the GNU C debug dll with C cleanup code)"
	@ echo "$(MAKE) clean GCE-debug             (to build the GNU C debug dll with C++ exception handling)"
	@ echo "$(MAKE) clean GC-inlined-debug      (to build the GNU C inlined debug dll with C cleanup code)"
	@ echo "$(MAKE) clean GCE-inlined-debug     (to build the GNU C inlined debug dll with C++ exception handling)"
	@ echo "$(MAKE) clean GC-static-debug       (to build the GNU C inlined static debug lib with C cleanup code)"
	@ echo "$(MAKE) clean GC-small-static-debug (to build the GNU C small static debug lib with C cleanup code)"

all:
	@ $(MAKE) clean GC-inlined
	@ $(MAKE) clean GCE-inlined
	@ $(MAKE) clean GC-static

TEST_ENV = PTW32_FLAGS="$(PTW32_FLAGS) -DNO_ERROR_DIALOGS" DLL_VER=$(DLL_VER)

all-tests:
	$(MAKE) realclean GC
	cd tests && $(MAKE) clean GC $(TEST_ENV) && $(MAKE) clean GCX $(TEST_ENV)
	$(MAKE) realclean GCE
	cd tests && $(MAKE) clean GCE $(TEST_ENV)
	$(MAKE) realclean GC-inlined
	cd tests && $(MAKE) clean GC $(TEST_ENV) && $(MAKE) clean GCX $(TEST_ENV)
	$(MAKE) realclean GCE-inlined
	cd tests && $(MAKE) clean GCE $(TEST_ENV)
	$(MAKE) realclean GC-static
	cd tests && $(MAKE) clean GC-static $(TEST_ENV)
	$(MAKE) realclean GC-small-static
	cd tests && $(MAKE) clean GC-static $(TEST_ENV)
	@ $(ECHO) "$@ completed successfully."

all-tests-cflags:
	$(MAKE) all-tests PTW32_FLAGS="-Wall -Wextra"
	@ $(ECHO) "$@ completed successfully."

GC:
		$(MAKE) CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" $(GC_DLL)

GC-debug:
		$(MAKE) CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_DLL)

GCE:
		$(MAKE) CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" $(GCE_DLL)

GCE-debug:
		$(MAKE) CC=$(CXX) CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_CXX -g -O0" $(GCED_DLL)

GC-inlined:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" $(GC_INLINED_STAMP)

GC-inlined-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_INLINED_STAMP)

GCE-inlined:
		$(MAKE) CC=$(CXX) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" $(GCE_INLINED_STAMP)

GCE-inlined-debug:
		$(MAKE) CC=$(CXX) XOPT="-DPTW32_BUILD_INLINED" CLEANUP=-D__CLEANUP_CXX XC_FLAGS="$(GCE_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_CXX -g -O0" $(GCED_INLINED_STAMP)

GC-static:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" $(GC_STATIC_STAMP)

GC-static-debug:
		$(MAKE) XOPT="-DPTW32_BUILD_INLINED -DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(DLL_INLINED_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_STATIC_STAMP)

GC-small-static:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(SMALL_STATIC_OBJS)" $(GC_SMALL_STATIC_STAMP)

GC-small-static-debug:
		$(MAKE) XOPT="-DPTW32_STATIC_LIB" CLEANUP=-D__CLEANUP_C XC_FLAGS="$(GC_CFLAGS)" OBJ="$(SMALL_STATIC_OBJS)" DLL_VER=$(DLL_VERD) OPT="-D__CLEANUP_C -g -O0" $(GCD_SMALL_STATIC_STAMP)

tests:
	@ cd tests
	@ $(MAKE) auto

%.pre: %.c
	$(CC) -E -o $@ $(CFLAGS) $^

%.s: %.c
	$(CC) -c $(CFLAGS) -DPTW32_BUILD_INLINED -Wa,-ahl $^ > $@

%.o: %.rc
	$(RC) $(RCFLAGS) $(CLEANUP) -o $@ -i $<

.SUFFIXES: .dll .rc .c .o

.c.o:;		 $(CC) -c -o $@ $(CFLAGS) $(XC_FLAGS) $<


$(GC_DLL) $(GCD_DLL): $(DLL_OBJS)
	$(CC) $(OPT) -shared -o $(GC_DLL) $(DLL_OBJS) $(LFLAGS)
	$(DLLTOOL) -z pthread.def $(DLL_OBJS)
	$(DLLTOOL) -k --dllname $@ --output-lib $(GC_LIB) --def $(PTHREAD_DEF)

$(GCE_DLL): $(DLL_OBJS)
	$(CC) $(OPT) -mthreads -shared -o $(GCE_DLL) $(DLL_OBJS) $(LFLAGS)
	$(DLLTOOL) -z pthread.def $(DLL_OBJS)
	$(DLLTOOL) -k --dllname $@ --output-lib $(GCE_LIB) --def $(PTHREAD_DEF)

$(GC_INLINED_STAMP) $(GCD_INLINED_STAMP): $(DLL_INLINED_OBJS)
	$(CC) $(OPT) $(XOPT) -shared -o $(GC_DLL) $(DLL_INLINED_OBJS) $(LFLAGS)
	$(DLLTOOL) -z pthread.def $(DLL_INLINED_OBJS)
	$(DLLTOOL) -k --dllname $(GC_DLL) --output-lib $(GC_LIB) --def $(PTHREAD_DEF)
	$(ECHO) touched > $(GC_INLINED_STAMP)

$(GCE_INLINED_STAMP) $(GCED_INLINED_STAMP): $(DLL_INLINED_OBJS)
	$(CC) $(OPT) $(XOPT) -mthreads -shared -o $(GCE_DLL) $(DLL_INLINED_OBJS)  $(LFLAGS)
	$(DLLTOOL) -z pthread.def $(DLL_INLINED_OBJS)
	$(DLLTOOL) -k --dllname $(GCE_DLL) --output-lib $(GCE_LIB) --def $(PTHREAD_DEF)
	$(ECHO) touched > $(GCE_INLINED_STAMP)

$(GC_STATIC_STAMP) $(GCD_STATIC_STAMP): $(DLL_INLINED_OBJS)
	$(RM) $(GC_LIB)
	$(AR) -rsv $(GC_LIB) $(DLL_INLINED_OBJS)
	$(ECHO) touched > $(GC_STATIC_STAMP)

$(GC_SMALL_STATIC_STAMP) $(GCD_SMALL_STATIC_STAMP): $(SMALL_STATIC_OBJS)
	$(RM) $(GC_LIB)
	$(AR) -rsv $(GC_LIB) $(SMALL_STATIC_OBJS)
	$(ECHO) touched > $(GC_SMALL_STATIC_STAMP)

clean:
	-$(RM) *~
	-$(RM) *.i
	-$(RM) *.s
	-$(RM) *.o
	-$(RM) *.obj
	-$(RM) *.exe
	-$(RM) $(PTHREAD_DEF)

realclean: clean
	-$(RM) $(GC_LIB)
	-$(RM) $(GCE_LIB)
	-$(RM) $(GC_DLL)
	-$(RM) $(GCE_DLL)
	-$(RM) $(GC_INLINED_STAMP)
	-$(RM) $(GCE_INLINED_STAMP)
	-$(RM) $(GC_STATIC_STAMP)
	-$(RM) $(GC_SMALL_STATIC_STAMP)
	-$(RM) $(GCD_LIB)
	-$(RM) $(GCED_LIB)
	-$(RM) $(GCD_DLL)
	-$(RM) $(GCED_DLL)
	-$(RM) $(GCD_INLINED_STAMP)
	-$(RM) $(GCED_INLINED_STAMP)
	-$(RM) $(GCD_STATIC_STAMP)
	-$(RM) $(GCD_SMALL_STATIC_STAMP)
	-cd tests && $(MAKE) clean

var_check_list =

define var_check_target
var-check-$(1):
	@for src in $($(1)); do \
		fgrep -q "\"$$$$src\"" $(2) && continue; \
		echo "$$$$src is in \$$$$($(1)), but not in $(2)"; \
		exit 1; \
	done
	@grep '^# *include *".*\.c"' $(2) | cut -d'"' -f2 | while read src; do \
		echo " $($(1)) " | fgrep -q " $$$$src " && continue; \
		echo "$$$$src is in $(2), but not in \$$$$($(1))"; \
		exit 1; \
	done
	@echo "$(1) <-> $(2): OK"

var_check_list += var-check-$(1)
endef

$(eval $(call var_check_target,ATTR_SRCS,attr.c))
$(eval $(call var_check_target,BARRIER_SRCS,barrier.c))
$(eval $(call var_check_target,CANCEL_SRCS,cancel.c))
$(eval $(call var_check_target,CONDVAR_SRCS,condvar.c))
$(eval $(call var_check_target,EXIT_SRCS,exit.c))
$(eval $(call var_check_target,MISC_SRCS,misc.c))
$(eval $(call var_check_target,MUTEX_SRCS,mutex.c))
$(eval $(call var_check_target,NONPORTABLE_SRCS,nonportable.c))
$(eval $(call var_check_target,PRIVATE_SRCS,private.c))
$(eval $(call var_check_target,RWLOCK_SRCS,rwlock.c))
$(eval $(call var_check_target,SCHED_SRCS,sched.c))
$(eval $(call var_check_target,SEMAPHORE_SRCS,semaphore.c))
$(eval $(call var_check_target,SPIN_SRCS,spin.c))
$(eval $(call var_check_target,SYNC_SRCS,sync.c))
$(eval $(call var_check_target,TSD_SRCS,tsd.c))

srcs-vars-check: $(var_check_list)

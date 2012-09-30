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

DLL_VER	= 2

CP	= cp -f
MV	= mv -f
RM	= rm -f
CAT	= cat
MKDIR	= mkdir
ECHO	= echo
TOUCH	= $(ECHO) Passed >
#MAKE	= make -k

# For cross compiling use e.g.
# # make CROSS=i386-mingw32msvc- clean GC
CROSS   =

# For cross testing use e.g.
# # make RUN=wine CROSS=i386-mingw32msvc- clean GC
RUN     =

AR      = $(CROSS)ar
DLLTOOL = $(CROSS)dlltool
CC      = $(CROSS)gcc
CXX     = $(CROSS)g++
RANLIB  = $(CROSS)ranlib

#
# Mingw
#
ARCH	= 

XLIBS	=
XXCFLAGS	= 
XXLIBS	=
OPT	= -O3
DOPT	= -g -O0
CFLAGS	= ${OPT} $(ARCH) -UNDEBUG -Wall $(XXCFLAGS)
LFLAGS	= $(XXLFLAGS)
# Uncomment to link the GCC/C++ runtime libraries statically
LFLAGS	+= -static-libgcc -static-libstdc++
BUILD_DIR	= ..
INCLUDES	= -I.

.INTERMEDIATE: %.exe %.pass
.SECONDARY: %.exe %.pass
.PRECIOUS: %.exe %.pass

TEST	= GC

# Default lib version
GCX	= GC$(DLL_VER)

# Files we need to run the tests
# - paths are relative to pthreads build dir.
HDR	= pthread.h semaphore.h sched.h
LIB	= libpthread$(GCX).a
DLL	= pthread$(GCX).dll
# The next path is relative to $BUILD_DIR
QAPC	=  # ../QueueUserAPCEx/User/quserex.dll

COPYFILES	= $(HDR) $(LIB) $(DLL) $(QAPC)

# If a test case returns a non-zero exit code to the shell, make will
# stop.

ALLTESTS = \
	sizes \
	self1 mutex5 mutex1 mutex1e mutex1n mutex1r \
	semaphore1 semaphore2 semaphore3 \
	condvar1 condvar1_1 condvar1_2 condvar2 condvar2_1 exit1 \
	create1 create2 create3 reuse1 reuse2 equal1 \
	sequence1 kill1 valid1 valid2 \
	exit2 exit3 exit4 exit5 \
	join0 join1 detach1 join2 join3 join4 \
	mutex2 mutex2r mutex2e mutex3 mutex3r mutex3e \
	mutex4 mutex6 mutex6n mutex6e mutex6r \
	mutex6s mutex6es mutex6rs \
	mutex7 mutex7n mutex7e mutex7r mutex8 mutex8n mutex8e mutex8r \
	robust1 robust2 robust3 robust4 robust5 \
	count1 \
	once1 once2 once3 once4 self2 \
	cancel1 cancel2 \
	semaphore4 semaphore4t semaphore5 \
	barrier1 barrier2 barrier3 barrier4 barrier5 barrier6 \
	tsd1 tsd2 delay1 delay2 eyal1 \
	condvar3 condvar3_1 condvar3_2 condvar3_3 \
	condvar4 condvar5 condvar6 condvar7 condvar8 condvar9 \
	errno1 \
	rwlock1 rwlock2 rwlock3 rwlock4 rwlock5 rwlock6 rwlock7 rwlock8 \
	rwlock2_t rwlock3_t rwlock4_t rwlock5_t rwlock6_t rwlock6_t2 \
	context1 cancel3 cancel4 cancel5 cancel6a cancel6d \
	cancel7 cancel8 \
	cleanup0 cleanup1 cleanup2 cleanup3 \
	priority1 priority2 inherit1 \
	spin1 spin2 spin3 spin4 \
	exception1 exception2 exception3_0 exception3 \
	cancel9 affinity1 affinity2 affinity3 affinity4 affinity5 \
	stress1

BENCHTESTS = \
	benchtest1 benchtest2 benchtest3 benchtest4 benchtest5

ASM				= $(ALLTESTS:%=%.s)
PASSES			= $(ALLTESTS:%=%.pass)
BENCHRESULTS	= $(BENCHTESTS:%=%.bench)
XTESTS			= loadfree.pass
TESTS			= $(PASSES) allpassed

help:
	@ $(ECHO) "Run one of the following command lines:"
	@ $(ECHO) "$(MAKE) clean GC                 (to test using GC dll with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCX                (to test using GC dll with C++ (EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCE                (to test using GCE dll with C++ (EH) applications)"
	@ $(ECHO) "$(MAKE) clean GC-bench           (to benchtest using GNU C dll with C cleanup code)"
	@ $(ECHO) "$(MAKE) clean GC-debug           (to test using GC dll with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GC-static          (to test using GC static lib with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GC-static-debug    (to test using GC static lib with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCE-static         (to test using GC static lib with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCE-static-debug   (to test using GC static lib with C (no EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCE-debug          (to test using GCE dll with C++ (EH) applications)"
	@ $(ECHO) "$(MAKE) clean GCX-static         (to test using GC static lib with C++ applications)"
	@ $(ECHO) "$(MAKE) clean GCX-static-debug   (to test using GC static lib with C++ applications)"
	@ $(ECHO) "$(MAKE) clean GCX-debug          (to test using GCE dll with C++ (EH) applications)"
	@ $(ECHO) "$(MAKE) clean GC-foo             (to build individual test \"foo.c\" with C and run using GC dll)"
	@ $(ECHO) "$(MAKE) clean GCE-foo            (to build individual tests \"foo.c\" with C++ and run using GCE dll)" 
	@ $(ECHO) "$(MAKE) clean GCX-foo            (to build individual tests \"foo.c\" with C++ and run using GC dll)"

GC:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CC) XXCFLAGS="-D__CLEANUP_C" $(TESTS)

GC-asm:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CC) XXCFLAGS="-D__CLEANUP_C" all-asm

GC-bench:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CC) XXCFLAGS="-D__CLEANUP_C" XXLIBS="benchlib.o" all-bench

GC-bench-debug:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)d" CC=$(CC) XXCFLAGS="-D__CLEANUP_C" XXLIBS="benchlib.o" OPT="${DOPT}" all-bench

GC-debug:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)d" CC=$(CC) XXCFLAGS="-D__CLEANUP_C" OPT="${DOPT}" $(TESTS)

GC-static GC-small-static:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CC) XXCFLAGS="-D__CLEANUP_C -DPTW32_STATIC_LIB" DLL="" XTESTS="" $(TESTS)

GC-static-debug GC-small-static-debug:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)d" CC=$(CC) XXCFLAGS="-D__CLEANUP_C -DPTW32_STATIC_LIB" OPT="$(DOPT)" DLL="" XTESTS="" $(TESTS)

GCE:
	@ $(MAKE) TEST=$@ GCX="GCE$(DLL_VER)" CC=$(CXX) XXCFLAGS="-mthreads -D__CLEANUP_CXX" $(TESTS)

GCE-debug:
	@ $(MAKE) TEST=$@ GCX="GCE$(DLL_VER)d" CC=$(CXX) XXCFLAGS="-D__CLEANUP_CXX" OPT="${DOPT}" $(TESTS)

GCE-static GCE-small-static:
	@ $(MAKE) TEST=$@ GCX="GCE$(DLL_VER)" CC=$(CXX) XXCFLAGS="-D__CLEANUP_CXX -DPTW32_STATIC_LIB" DLL="" XTESTS="" $(TESTS)

GCE-static-debug GCE-small-static-debug:
	@ $(MAKE) TEST=$@ GCX="GCE$(DLL_VER)d" CC=$(CXX) XXCFLAGS="-D__CLEANUP_CXX -DPTW32_STATIC_LIB" OPT="$(DOPT)" DLL="" XTESTS="" $(TESTS)

GCX:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CXX) XXCFLAGS="-mthreads -D__CLEANUP_C" $(TESTS)

GCX-debug:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)d" CC=$(CXX) XXCFLAGS="-D__CLEANUP_C" OPT="${DOPT}" $(TESTS)

GCX-static GCX-small-static:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)" CC=$(CXX) XXCFLAGS="-D__CLEANUP_C -DPTW32_STATIC_LIB" DLL="" XTESTS="" $(TESTS)

GCX-static-debug GCX-small-static-debug:
	@ $(MAKE) TEST=$@ GCX="GC$(DLL_VER)d" CC=$(CXX) XXCFLAGS="-D__CLEANUP_C -DPTW32_STATIC_LIB" OPT="$(DOPT)" DLL="" XTESTS="" $(TESTS)


all-asm: $(ASM)
	@ $(ECHO) "ALL TESTS PASSED! Congratulations!"

allpassed:
	@ $(ECHO) "ALL TESTS PASSED! Congratulations!"

all-bench: $(BENCHRESULTS)
	@ $(ECHO) "BENCH TESTS COMPLETED."

benchtest1.bench:
benchtest2.bench:
benchtest3.bench:
benchtest4.bench:
benchtest5.bench:

affinity1.pass:
affinity2.pass: affinity1.pass
affinity3.pass: affinity2.pass
affinity4.pass: affinity3.pass
affinity5.pass: affinity4.pass
barrier1.pass: semaphore4.pass
barrier2.pass: barrier1.pass
barrier3.pass: barrier2.pass
barrier4.pass: barrier3.pass
barrier5.pass: barrier4.pass
barrier6.pass: barrier5.pass
cancel1.pass: create1.pass
cancel2.pass: barrier6.pass
cancel3.pass: context1.pass
cancel4.pass: cancel3.pass
cancel5.pass: cancel3.pass
cancel6a.pass: cancel3.pass
cancel6d.pass: cancel3.pass
cancel7.pass: kill1.pass
cancel8.pass: cancel7.pass
cancel9.pass: cancel8.pass
cleanup0.pass: cancel5.pass
cleanup1.pass: cleanup0.pass
cleanup2.pass: cleanup1.pass
cleanup3.pass: cleanup2.pass
condvar1.pass:
condvar1_1.pass: condvar1.pass
condvar1_2.pass: join2.pass
condvar2.pass: condvar1.pass
condvar2_1.pass: condvar2.pass join2.pass
condvar3.pass: create1.pass condvar2.pass
condvar3_1.pass: condvar3.pass join2.pass
condvar3_2.pass: condvar3_1.pass
condvar3_3.pass: condvar3_2.pass
condvar4.pass: create1.pass
condvar5.pass: condvar4.pass
condvar6.pass: condvar5.pass
condvar7.pass: condvar6.pass cleanup1.pass
condvar8.pass: condvar7.pass
condvar9.pass: condvar8.pass
context1.pass: cancel1.pass
count1.pass: join1.pass
create1.pass: mutex2.pass
create2.pass: create1.pass
create3.pass: create2.pass
delay1.pass:
delay2.pass: delay1.pass
detach1.pass: join0.pass
equal1.pass: create1.pass
errno1.pass: mutex3.pass
exception1.pass: cancel4.pass
exception2.pass: exception1.pass
exception3_0.pass: exception2.pass
exception3.pass: exception3_0.pass
exit1.pass:
exit2.pass: create1.pass
exit3.pass: create1.pass
exit4.pass:
exit5.pass: exit4.pass kill1.pass
eyal1.pass: tsd1.pass
inherit1.pass: join1.pass priority1.pass
join0.pass: create1.pass
join1.pass: create1.pass
join2.pass: create1.pass
join3.pass: join2.pass
join4.pass: join3.pass
kill1.pass:
loadfree.pass:
mutex1.pass: self1.pass
mutex1n.pass: mutex1.pass
mutex1e.pass: mutex1.pass
mutex1r.pass: mutex1.pass
mutex2.pass: mutex1.pass
mutex2r.pass: mutex2.pass
mutex2e.pass: mutex2.pass
mutex3.pass: create1.pass
mutex3r.pass: mutex3.pass
mutex3e.pass: mutex3.pass
mutex4.pass: mutex3.pass
mutex5.pass:
mutex6.pass: mutex4.pass
mutex6n.pass: mutex4.pass
mutex6e.pass: mutex4.pass
mutex6r.pass: mutex4.pass
mutex6s.pass: mutex6.pass
mutex6rs.pass: mutex6r.pass
mutex6es.pass: mutex6e.pass
mutex7.pass: mutex6.pass
mutex7n.pass: mutex6n.pass
mutex7e.pass: mutex6e.pass
mutex7r.pass: mutex6r.pass
mutex8.pass: mutex7.pass
mutex8n.pass: mutex7n.pass
mutex8e.pass: mutex7e.pass
mutex8r.pass: mutex7r.pass
once1.pass: create1.pass
once2.pass: once1.pass
once3.pass: once2.pass
once4.pass: once3.pass
priority1.pass: join1.pass
priority2.pass: priority1.pass barrier3.pass
reuse1.pass: create3.pass
reuse2.pass: reuse1.pass
robust1.pass: mutex8r.pass
robust2.pass: mutex8r.pass
robust3.pass: robust2.pass
robust4.pass: robust3.pass
robust5.pass: robust4.pass
rwlock1.pass: condvar6.pass
rwlock2.pass: rwlock1.pass
rwlock3.pass: rwlock2.pass join2.pass
rwlock4.pass: rwlock3.pass
rwlock5.pass: rwlock4.pass
rwlock6.pass: rwlock5.pass
rwlock7.pass: rwlock6.pass
rwlock8.pass: rwlock7.pass
rwlock2_t.pass: rwlock2.pass
rwlock3_t.pass: rwlock2_t.pass
rwlock4_t.pass: rwlock3_t.pass
rwlock5_t.pass: rwlock4_t.pass
rwlock6_t.pass: rwlock5_t.pass
rwlock6_t2.pass: rwlock6_t.pass
self1.pass:
self2.pass: create1.pass
semaphore1.pass:
semaphore2.pass:
semaphore3.pass: semaphore2.pass
semaphore4.pass: semaphore3.pass cancel1.pass
semaphore4t.pass: semaphore4.pass
semaphore5.pass: semaphore4.pass
sequence1.pass: reuse2.pass
sizes.pass:
spin1.pass:
spin2.pass: spin1.pass
spin3.pass: spin2.pass
spin4.pass: spin3.pass
stress1.pass:
tsd1.pass: barrier5.pass join1.pass
tsd2.pass: tsd1.pass
valid1.pass: join1.pass
valid2.pass: valid1.pass

loadfree.pass: pthread.dll loadfree.exe
	@ $(ECHO) Running $(TEST) test \"loadfree\"
	@ $(RUN) ./loadfree.exe
	@ $(ECHO) Passed
	@ $(TOUCH) $@

sizes.pass: sizes.exe
	@ $(ECHO) Running $(TEST) test \"sizes\"
	@ $(RUN) ./sizes.exe > SIZES.$(TEST)
	@ $(CAT) SIZES.$(TEST)
	@ $(ECHO) Passed
	@ $(TOUCH) $@

%.pass: sizes.pass $(XTESTS) %.exe
	@ $(TOUCH) $@

%.bench:  $(COPYFILES) $(XXLIBS) %.exe
	@ $(ECHO) Running $(TEST) test \"$*\"
	@ $(RUN) ./$*
	@ $(ECHO) Done
	@ $(TOUCH) $@

loadfree.exe: loadfree.c $(COPYFILES)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $< $(INCLUDES) -L. -lpthread$(GCX) $(XLIBS) $(XXLIBS)	

sizes.exe: sizes.c $(COPYFILES)
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $< $(INCLUDES) -L. -lpthread$(GCX) $(XLIBS) $(XXLIBS)	

cancel9.exe: XLIBS = -lws2_32

%.exe: %.c
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $< $(INCLUDES) -L. -lpthread$(GCX) $(XLIBS) $(XXLIBS)	
	@ $(ECHO) Running $(TEST) test \"$*\"
	@ $(RUN) ./$*
	@ $(ECHO) Passed

%.pre: %.c $(HDR)
	$(CC) -E $(CFLAGS) -o $@ $< $(INCLUDES)

%.s: %.c $(HDR)
	@ $(ECHO) Compiling $@
	$(CC) -S $(CFLAGS) -o $@ $< $(INCLUDES)

$(COPYFILES):
	@ $(ECHO) Copying $(BUILD_DIR)/$@
	@ $(CP) $(BUILD_DIR)/$@ .

benchlib.o: benchlib.c
	@ $(ECHO) Compiling $@
	$(CC) -c $(CFLAGS) $< $(INCLUDES)

pthread.dll: $(DLL)
	@ $(CP) $(DLL) $@

clean:
	- $(RM) *.dll
	- $(RM) *.lib
	- $(RM) pthread.h
	- $(RM) semaphore.h
	- $(RM) sched.h
	- $(RM) *.a
	- $(RM) *.e
	- $(RM) *.i
	- $(RM) *.o
	- $(RM) *.s
	- $(RM) *.so
	- $(RM) *.obj
	- $(RM) *.pdb
	- $(RM) *.exe
	- $(RM) *.manifest
	- $(RM) *.pass
	- $(RM) *.bench
	- $(RM) *.log
	- $(RM) SIZES.G*
	
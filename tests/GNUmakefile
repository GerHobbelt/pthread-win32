# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.
#
# --------------------------------------------------------------------------
#
#      Pthreads-win32 - POSIX Threads Library for Win32
#      Copyright(C) 1998 John E. Bossom
#      Copyright(C) 1999,2003 Pthreads-win32 contributors
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


CP	= cp -f
MV	= mv -f
RM	= rm -f
#CP	= copy
#MV	= rename
#RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo
MAKE	= make

#
# Mingw32
#
XXCFLAGS	= 
CFLAGS	= -O3 -UNDEBUG -Wall $(XXCFLAGS)
#CFLAGS	= -g -O0 -UNDEBUG -Wall $(XXCFLAGS)
BUILD_DIR	= ..
INCLUDES	= -I.

GCX	= DUMMY
HDR	= pthread.h semaphore.h sched.h
LIB	= libpthread$(GCX).a
DLL	= pthread$(GCX).dll

COPYFILES	= $(HDR) $(LIB) $(DLL)

# If a test case returns a non-zero exit code to the shell, make will
# stop.

TESTS	= loadfree \
	  semaphore1 semaphore2 self1 mutex5 mutex1 mutex1e mutex1n mutex1r \
	  condvar1 condvar1_1 condvar1_2 condvar2 condvar2_1 exit1 \
	  create1 create2 reuse1 reuse2 equal1 \
	  kill1 valid1 valid2 \
	  exit2 exit3 \
	  join0 join1 join2 \
	  mutex2 mutex3 mutex4 mutex6 mutex6n mutex6e mutex6r \
	  mutex7 mutex7n mutex7e mutex7r mutex8 mutex8n mutex8e mutex8r \
	  count1 once1 tsd1 self2 cancel1 cancel2 \
	  delay1 delay2 eyal1 \
	  condvar3 condvar3_1 condvar3_2 condvar3_3 \
	  condvar4 condvar5 condvar6 condvar7 condvar8 condvar9 \
	  errno1 \
	  rwlock1 rwlock2 rwlock3 rwlock4 rwlock5 rwlock6 rwlock7 \
	  rwlock2_t rwlock3_t rwlock4_t rwlock5_t rwlock6_t rwlock6_t2 \
	  context1 cancel3 cancel4 cancel5 cancel6a cancel6d \
	  cleanup0 cleanup1 cleanup2 cleanup3 \
	  priority1 priority2 inherit1 \
	  spin1 spin2 spin3 spin4 \
	  barrier1 barrier2 barrier3 barrier4 barrier5 \
	  exception1 exception2 exception3

BENCHTESTS = \
	benchtest1 benchtest2 benchtest3 benchtest4 benchtest5

PASSES		= $(TESTS:%=%.pass)
BENCHRESULTS	= $(BENCHTESTS:%=%.bench)

help:
	@ $(ECHO) "Run one of the following command lines:"
	@ $(ECHO) "make clean GC    (to test using GC dll with C (no EH) applications)"
	@ $(ECHO) "make clean GCX   (to test using GC dll with C++ (EH) applications)"
	@ $(ECHO) "make clean GCE   (to test using GCE dll with C++ (EH) applications)"
	@ $(ECHO) "make clean GC-bench	  (to benchtest using GNU C dll with C cleanup code)"
	@ $(ECHO) "make clean GCE-bench   (to benchtest using GNU C dll with C++ exception handling)"

all:
	@ $(MAKE) clean GC
	@ $(MAKE) clean GCX
	@ $(MAKE) clean GCE

GC:
	$(MAKE) GCX=GC CC=gcc XXCFLAGS="-D__CLEANUP_C" all-pass

GCE:
	$(MAKE) GCX=GCE CC=g++ XXCFLAGS="-mthreads -D__CLEANUP_CXX" all-pass

GCX:
	$(MAKE) GCX=GC CC=g++ XXCFLAGS="-mthreads -D__CLEANUP_C" all-pass

GC-bench:
	$(MAKE) GCX=GC CC=gcc XXCFLAGS="-D__CLEANUP_C" all-bench

GCE-bench:
	$(MAKE) GCX=GCE  CC=g++ XXCFLAGS="-mthreads -D__CLEANUP_CXX" all-bench

all-pass: $(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

all-bench: $(BENCHRESULTS)
	@ $(ECHO) BENCH TESTS COMPLETED.

benchtest1.bench:
benchtest2.bench:
benchtest3.bench:
benchtest4.bench:
benchtest5.bench:

barrier1.pass:
barrier2.pass: barrier1.pass
barrier3.pass: barrier2.pass
barrier4.pass: barrier3.pass
barrier5.pass: barrier4.pass
cancel1.pass: create1.pass
cancel2.pass: cancel1.pass
cancel2_1.pass: cancel2.pass
cancel3.pass: context1.pass
cancel4.pass: cancel3.pass
cancel5.pass: cancel3.pass
cancel6a.pass: cancel3.pass
cancel6d.pass: cancel3.pass
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
context1.pass: cancel2.pass
count1.pass: join1.pass
create1.pass: mutex2.pass
create2.pass: create1.pass
delay1.pass: cancel2.pass
delay2.pass: delay1.pass
equal1.pass: create1.pass
errno1.pass: mutex3.pass
exception1.pass: cancel4.pass
exception2.pass: exception1.pass
exception3.pass: exception2.pass
exit1.pass:
exit2.pass: create1.pass
exit3.pass: create1.pass
eyal1.pass: tsd1.pass
inherit1.pass: join1.pass
join0.pass: create1.pass
join1.pass: create1.pass
join2.pass: create1.pass
kill1.pass:
loadfree.pass: pthread.dll
mutex1.pass: self1.pass
mutex1n.pass: mutex1.pass
mutex1e.pass: mutex1.pass
mutex1r.pass: mutex1.pass
mutex2.pass: mutex1.pass
mutex3.pass: create1.pass
mutex4.pass: mutex3.pass
mutex5.pass:
mutex6.pass: mutex4.pass
mutex6n.pass: mutex4.pass
mutex6e.pass: mutex4.pass
mutex6r.pass: mutex4.pass
mutex7.pass: mutex6.pass
mutex7n.pass: mutex6n.pass
mutex7e.pass: mutex6e.pass
mutex7r.pass: mutex6r.pass
mutex8.pass: mutex7.pass
mutex8n.pass: mutex7n.pass
mutex8e.pass: mutex7e.pass
mutex8r.pass: mutex7r.pass
once1.pass: create1.pass
priority1.pass: join1.pass
priority2.pass: priority1.pass
reuse1.pass: create2.pass
reuse2.pass: reuse1.pass
rwlock1.pass: condvar6.pass
rwlock2.pass: rwlock1.pass
rwlock3.pass: rwlock2.pass
rwlock4.pass: rwlock3.pass
rwlock5.pass: rwlock4.pass
rwlock6.pass: rwlock5.pass
rwlock7.pass: rwlock6.pass
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
spin1.pass:
spin2.pass: spin1.pass
spin3.pass: spin2.pass
spin4.pass: spin3.pass
tsd1.pass: join1.pass
valid1.pass: join1.pass
valid2.pass: valid1.pass

#%.pass: %.exe $(HDR)
%.pass: %.exe $(LIB) $(DLL) $(HDR)
	@ $(ECHO) Running $*
	$*
	@ $(ECHO) Passed
	@ $(TOUCH) $@

%.bench: %.exe $(LIB) $(DLL) $(HDR)
	@ $(ECHO) Running $*
	$*
	@ $(ECHO) Done
	@ $(TOUCH) $@

%.exe: %.c
	@ $(ECHO) Compiling $@
	@ $(ECHO) $(CC) $(CFLAGS) -o $@ $^ $(INCLUDES) -L. -lpthread$(GCX)
	@ $(CC) $(CFLAGS) -o $@ $^ $(INCLUDES) -L. -lpthread$(GCX) -lsupc++

%.pre: %.c
	@ $(CC) -E $(CFLAGS) -o $@ $^ $(INCLUDES)

%.s: %.c
	@ $(CC) -S $(CFLAGS) -o $@ $^ $(INCLUDES)

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)/$@ .

pthread.dll:
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
	- $(RM) *.obj
	- $(RM) *.pdb
	- $(RM) *.exe
	- $(RM) *.pass
	- $(RM) *.bench
	- $(RM) *.log

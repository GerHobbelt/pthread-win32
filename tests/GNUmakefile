# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


#CP	= cp
#MV	= mv
#RM	= rm
CP	= copy
MV	= rename
RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo
MAKE	= make

#
# Mingw32
#
GLANG   = c++
CC	= gcc
XXCFLAGS	= 
#CFLAGS	= -g -O0 -mthreads -UNDEBUG -Wall -x $(GLANG)
#CFLAGS	= -O3 -mthreads -UNDEBUG -Wall -x $(GLANG)
CFLAGS	= -g -O0 -UNDEBUG -Wall $(XXCFLAGS)
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
	  self1 mutex5 mutex1 mutex1e mutex1n mutex1r \
	  condvar1 condvar2 condvar2_1 exit1 create1 equal1 \
	  exit2 exit3 \
	  join0 join1 join2 mutex2 mutex3 mutex4 mutex6 mutex6n mutex6e mutex6r \
	  count1 once1 tsd1 self2 cancel1 cancel2 eyal1 \
	  condvar3 condvar3_1 condvar3_2 condvar3_3 \
	  condvar4 condvar5 condvar6 condvar7 condvar8 condvar9 \
	  errno1 \
	  rwlock1 rwlock2 rwlock3 rwlock4 rwlock5 rwlock6 rwlock7 \
	  context1 cancel3 cancel4 cancel5 \
	  cleanup0 cleanup1 cleanup2 cleanup3 \
	  priority1 priority2 inherit1 \
	  spin1 spin2 spin3 spin4 \
	  barrier1 barrier2 barrier3 barrier4 barrier5 \
	  exception1 exception2 exception3

BENCHTESTS = \
	benchtest1 benchtest2 benchtest3 benchtest4

PASSES		= $(TESTS:%=%.pass)
BENCHRESULTS	= $(BENCHTESTS:%=%.bench)

default:
	@ $(ECHO) Run one of the following command lines:
	@ $(ECHO) nmake clean GCE   (to test using GNU C dll with C++ exception handling)
	@ $(ECHO) nmake clean GC    (to test using GNU C dll with C cleanup code)
	@ $(ECHO) nmake clean GCE-bench   (to benchtest using GNU C dll with C++ exception handling)
	@ $(ECHO) nmake clean GC-bench    (to benchtest using GNU C dll with C cleanup code)

auto:
	@ $(MAKE) clean GCE
	@ $(MAKE) clean GC

GC:
	$(MAKE) GCX=GC XXCFLAGS="-x c" all-pass

GCE:
	$(MAKE) GCX=GCE XXCFLAGS="-mthreads -x c++" all-pass

GC-bench:
	$(MAKE) GCX=GC XXCFLAGS="-x c" all-bench

GCE-bench:
	$(MAKE) GCX=GCE XXCFLAGS="-mthreads -x c++" all-bench

all-pass: $(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

all-bench: $(BENCHRESULTS)
	@ $(ECHO) BENCH TESTS COMPLETED.

benchtest1.bench:
benchtest2.bench:
benchtest3.bench:
benchtest4.bench:

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
cleanup0.pass: cancel5.pass
cleanup1.pass: cleanup0.pass
cleanup2.pass: cleanup1.pass
cleanup3.pass: cleanup2.pass
condvar1.pass:
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
loadfree.pass: pthread.dll
mutex1.pass: self1.pass
mutex1n.pass: mutex1.pass
mutex1e.pass: mutex1.pass
mutex1r.pass: mutex1.pass
mutex2.pass: mutex1.pass
mutex3.pass: create1.pass
mutex4.pass: mutex3.pass
mutex5.pass:
once1.pass: create1.pass
priority1.pass: join1.pass
priority2.pass: priority1.pass
rwlock1.pass: condvar6.pass
rwlock2.pass: rwlock1.pass
rwlock3.pass: rwlock2.pass
rwlock4.pass: rwlock3.pass
rwlock5.pass: rwlock4.pass
rwlock6.pass: rwlock5.pass
rwlock7.pass: rwlock6.pass
self1.pass:
self2.pass: create1.pass
spin1.pass:
spin2.pass: spin1.pass
spin3.pass: spin2.pass
spin4.pass: spin3.pass
tsd1.pass: join1.pass

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
	@ $(CC) $(CFLAGS) -o $@ $^ $(INCLUDES) -L. -lpthread$(GCX)

%.pre: %.c
	@ $(CC) -E $(CFLAGS) -o $@ $^ $(INCLUDES)

%.s: %.c
	@ $(CC) -S $(CFLAGS) -o $@ $^ $(INCLUDES)

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)\$@ .

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
	- $(RM) *.obj
	- $(RM) *.pdb
	- $(RM) *.exe
	- $(RM) *.pass
	- $(RM) *.bench

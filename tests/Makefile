# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


CP	= copy
RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo

CPHDR	= pthread.h semaphore.h sched.h

# C++ Exceptions
VCEFLAGS	= /GX /TP /DPtW32NoCatchWarn /D__CLEANUP_CXX
VCELIB	= pthreadVCE.lib
VCEDLL	= pthreadVCE.dll
# Structured Exceptions
VSEFLAGS	= /D__CLEANUP_SEH
VSELIB	= pthreadVSE.lib
VSEDLL	= pthreadVSE.dll
#C cleanup code
VCFLAGS	= /D__CLEANUP_C
VCLIB	= pthreadVC.lib
VCDLL	= pthreadVC.dll

CFLAGS= /W3 /WX /MT /nologo /Yd /Zi -D_WIN32_WINNT=0x400
LFLAGS= /INCREMENTAL:NO
INCLUDES=-I.
BUILD_DIR=..

COPYFILES	= $(CPHDR) $(CPLIB) $(CPDLL)

TEST		=
EHFLAGS	=

# If a test case returns a non-zero exit code to the shell, make will
# stop.

PASSES= loadfree.pass \
	  self1.pass mutex5.pass  \
	  mutex1.pass  mutex1n.pass  mutex1e.pass  mutex1r.pass  mutex2.pass  mutex3.pass  \
	  condvar1.pass  condvar2.pass  condvar2_1.pass  \
	  exit1.pass  create1.pass  equal1.pass  \
	  exit2.pass  exit3.pass  \
	  join0.pass  join1.pass  join2.pass  \
	  mutex4.pass  mutex6.pass  mutex6n.pass  mutex6e.pass  mutex6r.pass  \
	  count1.pass  once1.pass  tsd1.pass  \
	  self2.pass  \
	  cancel1.pass  cancel2.pass  \
	  eyal1.pass  \
	  condvar3.pass  condvar3_1.pass  condvar3_2.pass  condvar3_3.pass  \
	  condvar4.pass  condvar5.pass  condvar6.pass  \
	  condvar7.pass  condvar8.pass  condvar9.pass  \
	  errno1.pass  \
	  rwlock1.pass  rwlock2.pass  rwlock3.pass  rwlock4.pass  rwlock5.pass  rwlock6.pass  rwlock7.pass  \
	  context1.pass  \
	  cancel3.pass  cancel4.pass  cancel5.pass  \
	  cleanup0.pass  cleanup1.pass  cleanup2.pass  cleanup3.pass  \
	  priority1.pass priority2.pass inherit1.pass  \
	  exception1.pass  exception2.pass  exception3.pass

BENCHRESULTS = \
	  benchtest1.bench benchtest2.bench benchtest3.bench benchtest4.bench

all:
	@ $(ECHO) Run one of the following command lines:
	@ $(ECHO) nmake clean VCE   (to test using MSVC dll with C++ exception handling)
	@ $(ECHO) nmake clean VSE   (to test using MSVC dll with structured exception handling)
	@ $(ECHO) nmake clean VC    (to test using MSVC dll with C cleanup code)
	@ $(ECHO) nmake clean VCE-bench   (to benchtest using MSVC dll with C++ exception handling)
	@ $(ECHO) nmake clean VSE-bench   (to benchtest using MSVC dll with structured exception handling)
	@ $(ECHO) nmake clean VC-bench    (to benchtest using MSVC dll with C cleanup code)

auto:
	@ nmake clean VCE
	@ nmake clean VSE
	@ nmake clean VC

tests: $(CPLIB) $(CPDLL) $(CPHDR) $(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

benchtests: $(CPLIB) $(CPDLL) $(CPHDR) $(BENCHRESULTS)
	@ $(ECHO) ALL BENCH TESTS DONE.

$(PASSES): $*.exe
	@ $(ECHO) ... Running $(TEST) test: $*.exe
	@ .\$*.exe
	@ $(ECHO) ...... Passed
	@ $(TOUCH) $*.pass

$(BENCHRESULTS): $*.exe
	@ $(ECHO) ... Running $(TEST) benchtest: $*.exe
	@ .\$*.exe
	@ $(ECHO) ...... Done
	@ $(TOUCH) $*.bench

VCE:
	@ nmake TEST="$@" CPLIB="$(VCELIB)" CPDLL="$(VCEDLL)" EHFLAGS="$(VCEFLAGS)" tests

VSE:	
	@ nmake TEST="$@" CPLIB="$(VSELIB)" CPDLL="$(VSEDLL)" EHFLAGS="$(VSEFLAGS)" tests

VC:
	@ nmake TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCFLAGS)" tests

VCE-bench:
	@ nmake TEST="$@" CPLIB="$(VCELIB)" CPDLL="$(VCEDLL)" EHFLAGS="$(VCEFLAGS)" benchtests

VSE-bench:
	@ nmake TEST="$@" CPLIB="$(VSELIB)" CPDLL="$(VSEDLL)" EHFLAGS="$(VSEFLAGS)" benchtests

VC-bench:
	@ nmake TEST="$@" CPLIB="$(VCLIB)" CPDLL="$(VCDLL)" EHFLAGS="$(VCFLAGS)" benchtests

.c.exe:
	@ $(ECHO) Compiling $@
	@ $(CC) $(EHFLAGS) $(CFLAGS) $(INCLUDES) $< /Fe$@ /link $(LFLAGS) $(CPLIB)

.c.pre:
	@ $(CC) /E $(EHFLAGS) $(CFLAGS) $(INCLUDES) /Fe$@ $<

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)\$@ .

pthread.dll:
	@ $(CP) $(CPDLL) $*.dll
	@ $(CP) $(CPLIB) $*.lib

clean:
	- $(RM) *.dll
	- $(RM) *.lib
	- $(RM) pthread.h
	- $(RM) semaphore.h
	- $(RM) sched.h
	- $(RM) *.e
	- $(RM) *.obj
	- $(RM) *.pdb
	- $(RM) *.o
	- $(RM) *.exe
	- $(RM) *.pass
	- $(RM) *.bench

benchtest1.bench:
benchtest2.bench:
benchtest3.bench:
benchtest4.bench:
cancel1.pass: create1.pass
cancel2.pass: cancel1.pass
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
tsd1.pass: join1.pass

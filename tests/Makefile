# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


CP	= copy
RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo

CPHDR	= pthread.h semaphore.h sched.h

# C++ Exceptions
VCEFLAGS	= /GX /TP /DPtW32NoCatchWarn
VCELIB	= pthreadVCE.lib
VCEDLL	= pthreadVCE.dll
# Structured Exceptions
VSEFLAGS	=
VSELIB	= pthreadVSE.lib
VSEDLL	= pthreadVSE.dll

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
	  mutex1.pass  mutex2.pass  mutex3.pass mutex4.pass \
	  condvar1.pass  condvar2.pass  \
	  exit1.pass  create1.pass  equal1.pass  \
	  exit2.pass  exit3.pass  \
	  join0.pass  join1.pass  join2.pass  \
	  count1.pass  once1.pass  tsd1.pass  \
	  self1.pass  self2.pass  \
	  cancel1.pass  cancel2.pass  \
	  eyal1.pass  \
	  condvar3.pass  condvar4.pass  condvar5.pass  condvar6.pass  \
	  condvar7.pass  condvar8.pass  condvar9.pass  \
	  errno1.pass  \
	  rwlock1.pass  rwlock2.pass  rwlock3.pass  rwlock4.pass  rwlock5.pass  rwlock6.pass  rwlock7.pass \
	  context1.pass  \
	  cancel3.pass  cancel4.pass  cancel5.pass  \
	  cleanup0.pass  cleanup1.pass  cleanup2.pass  cleanup3.pass  \
	  exception1.pass  exception2.pass exception3.pass

all:
	@ $(ECHO) Run one of the following command lines:
	@ $(ECHO) nmake clean VCE   (to test using dll with C++ exception handling)
	@ $(ECHO) nmake clean VSE   (to test using dll with structured exception handling)

auto: clean VCE clean VSE

tests: $(CPLIB) $(CPDLL) $(CPHDR) $(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

$(PASSES): $*.exe
	@ $(ECHO) ... Running $(TEST) test: $*.exe
	@ .\$*.exe
	@ $(ECHO) ...... Passed
	@ $(TOUCH) $*.pass

VCE:
	@ nmake TEST="$@" CPLIB="$(VCELIB)" CPDLL="$(VCEDLL)" EHFLAGS="$(VCEFLAGS)" tests

VSE:	
	@ nmake TEST="$@" CPLIB="$(VSELIB)" CPDLL="$(VSEDLL)" EHFLAGS="$(VSEFLAGS)" tests

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

loadfree.pass: pthread.dll
mutex1.pass:
mutex2.pass:
exit1.pass:
condvar1.pass:
self1.pass:
condvar2.pass: condvar1.pass
create1.pass: mutex2.pass
cancel1.pass: create1.pass
cancel2.pass: cancel1.pass
mutex3.pass: create1.pass
mutex4.pass: mutex3.pass
equal1.pass: create1.pass
exit2.pass: create1.pass
exit3.pass: create1.pass
join0.pass: create1.pass
join1.pass: create1.pass
join2.pass: create1.pass
count1.pass: join1.pass
once1.pass: create1.pass
tsd1.pass: join1.pass
self2.pass: create1.pass
eyal1.pass: tsd1.pass
condvar3.pass: create1.pass
condvar4.pass: create1.pass
condvar5.pass: condvar4.pass
condvar6.pass: condvar5.pass
condvar7.pass: condvar6.pass cleanup1.pass
condvar8.pass: condvar7.pass
condvar9.pass: condvar8.pass
errno1.pass: mutex3.pass
rwlock1.pass: condvar6.pass
rwlock2.pass: rwlock1.pass
rwlock3.pass: rwlock2.pass
rwlock4.pass: rwlock3.pass
rwlock5.pass: rwlock4.pass
rwlock6.pass: rwlock5.pass
rwlock7.pass: rwlock6.pass
context1.pass: cancel2.pass
cancel3.pass: context1.pass
cancel4.pass: cancel3.pass
cancel5.pass: cancel3.pass
cleanup0.pass: cancel5.pass
cleanup1.pass: cleanup0.pass
cleanup2.pass: cleanup1.pass
cleanup3.pass: cleanup2.pass
exception1.pass: cancel4.pass
exception2.pass: exception1.pass
exception3.pass: exception2.pass

# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


CP	= copy
RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo

CPHDR	= pthread.h semaphore.h sched.h
CPLIB	= pthread.lib
CPDLL	= pthread.dll

CFLAGS= /W3 /MT /nologo /Yd /Zi -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall
LFLAGS= /INCREMENTAL:NO
LIBS	= .\pthread.lib
INCLUDES=-I.
BUILD_DIR=..

COPYFILES	= $(CPHDR) $(CPLIB) $(CPDLL)

# If a test case returns a non-zero exit code to the shell, make will
# stop.

PASSES= loadfree.pass \
	  mutex1.pass  condvar1.pass  condvar2.pass  \
	  exit1.pass  create1.pass  equal1.pass  \
	  exit2.pass  exit3.pass  \
	  join0.pass  join1.pass  join2.pass  \
	  mutex2.pass  mutex3.pass  \
	  count1.pass  once1.pass  tsd1.pass  \
	  self1.pass  self2.pass  \
	  cancel1.pass  cancel2.pass  \
	  eyal1.pass  \
	  condvar3.pass  condvar4.pass  condvar5.pass  condvar6.pass  \
	  condvar7.pass  condvar8.pass  condvar9.pass  \
	  errno1.pass  \
	  rwlock1.pass  rwlock2.pass  rwlock3.pass  rwlock4.pass  rwlock5.pass  rwlock6.pass  \
	  context1.pass  \
	  cancel3.pass  cancel4.pass  cleanup1.pass  \
	  cleanup2.pass  cleanup3.pass  \
	  exception1.pass 

all:	$(CPLIB) $(CPDLL) $(CPHDR) $(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

loadfree.pass: loadfree.exe
mutex1.pass: mutex1.exe
mutex2.pass: mutex2.exe
exit1.pass: exit1.exe
condvar1.pass: condvar1.exe
self1.pass: self1.exe
condvar2.pass: condvar2.exe condvar1.pass
create1.pass: create1.exe mutex2.pass
cancel1.pass: cancel1.exe create1.pass
cancel2.pass: cancel2.exe cancel1.pass
mutex3.pass: mutex3.exe create1.pass
equal1.pass: equal1.exe create1.pass
exit2.pass: exit2.exe create1.pass
exit3.pass: exit3.exe create1.pass
join0.pass: join0.exe create1.pass
join1.pass: join1.exe create1.pass
join2.pass: join2.exe create1.pass
count1.pass: count1.exe join1.pass
once1.pass: once1.exe create1.pass
tsd1.pass: tsd1.exe join1.pass
self2.pass: self2.exe create1.pass
eyal1.pass: eyal1.exe tsd1.pass
condvar3.pass: condvar3.exe create1.pass
condvar4.pass: condvar4.exe create1.pass
condvar5.pass: condvar5.exe condvar4.pass
condvar6.pass: condvar6.exe condvar5.pass
condvar7.pass: condvar7.exe condvar6.pass cleanup1.pass
condvar8.pass: condvar8.exe condvar7.pass
condvar9.pass: condvar9.exe condvar8.pass
errno1.pass: errno1.exe mutex3.pass
rwlock1.pass: rwlock1.exe condvar6.pass
rwlock2.pass: rwlock2.exe rwlock1.pass
rwlock3.pass: rwlock3.exe rwlock2.pass
rwlock4.pass: rwlock4.exe rwlock3.pass
rwlock5.pass: rwlock5.exe rwlock4.pass
rwlock6.pass: rwlock6.exe rwlock5.pass
context1.pass: context1.exe cancel2.pass
cancel3.pass: cancel3.exe context1.pass
cancel4.pass: cancel4.exe cancel3.pass
cleanup1.pass: cleanup1.exe cancel4.pass
cleanup2.pass: cleanup2.exe cleanup1.pass
cleanup3.pass: cleanup3.exe cleanup2.pass
exception1.pass: exception1.exe cancel4.pass

$(PASSES): $*.exe
	@ .\$*.exe
	@ $(ECHO) Passed
	@ $(TOUCH) $*.pass

.c.exe:
	@ $(CC) $(CFLAGS) $(INCLUDES) $< /Fe$@ /link $(LFLAGS) $(LIBS)

.c.pre:
	@ $(CC) /E $(CFLAGS) $(INCLUDES) /Fe$@ $<

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)\$@ .

clean:
	- $(RM) *.dll
	- $(RM) pthread.h
	- $(RM) semaphore.h
	- $(RM) sched.h
	- $(RM) *.lib
	- $(RM) *.e
	- $(RM) *.obj
	- $(RM) *.pdb
	- $(RM) *.o
	- $(RM) *.exe
	- $(RM) *.pass


# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


CP	= copy
RM	= erase
MKDIR	= mkdir
TOUCH	= echo Passed >
ECHO	= @echo

#
# Mingw32
#
GLANG   = c++
CC	= gcc
CFLAGS	= -g -O2 -mthreads -UNDEBUG -Wall -x $(GLANG)
BUILD_DIR	= ..
INCLUDES	= -I.
LIBS	= -L. -lpthread32

HDR	= pthread.h semaphore.h sched.h
LIB	= libpthread32.a
DLL	= pthread.dll

COPYFILES	= $(HDR) $(LIB) $(DLL)

# If a test case returns a non-zero exit code to the shell, make will
# stop.

TESTS	= loadfree \
	  mutex1 condvar1 condvar2 exit1 create1 equal1 \
	  exit2 exit3 \
	  join0 join1 join2 mutex2 mutex3 \
	  count1 once1 tsd1 self1 self2 cancel1 eyal1 \
	  condvar3 condvar4 condvar5 condvar6 condvar7 condvar8 condvar9 \
	  errno1 \
	  rwlock1 rwlock2 rwlock3 rwlock4 rwlock5 rwlock6 \
	  context1 cancel3 cancel4 cleanup1 cleanup2 cleanup3 \
	  exception1

PASSES	= $(TESTS:%=%.pass)

all:	$(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

loadfree.pass:
mutex1.pass:
mutex2.pass:
exit1.pass:
condvar1.pass:
self1.pass:
condvar2.pass: condvar1.pass
create1.pass: mutex2.pass
mutex3.pass: create1.pass
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
context1.pass: cancel2.pass
cancel3.pass: context1.pass
cancel4.pass: cancel3.pass
cleanup1.pass: cancel4.pass
cleanup2.pass: cleanup1.pass
cleanup3.pass: cleanup2.pass
exception1.pass: cancel4.pass

%.pass: %.exe $(LIB) $(DLL) $(HDR)
	$*
	@ $(ECHO) Passed
	@ $(TOUCH) $@

%.exe: %.c
	@ $(CC) $(CFLAGS) -o $@ $^ $(INCLUDES) $(LIBS)

%.pre: %.c
	@ $(CC) -E $(CFLAGS) -o $@ $^ $(INCLUDES)

%.s: %.c
	@ $(CC) -S $(CFLAGS) -o $@ $^ $(INCLUDES)

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)\$@ .

clean:
	- $(RM) *.dll
	- $(RM) pthread.h
	- $(RM) semaphore.h
	- $(RM) sched.h
	- $(RM) *.a
	- $(RM) *.e
	- $(RM) *.exe
	- $(RM) *.pass


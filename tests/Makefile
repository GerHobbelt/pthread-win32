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
GLANG	= c
CC	= gcc
CFLAGS	= -g -O2 -UNDEBUG -Wall -x $(GLANG) -o $@ $^
BUILD_DIR	= ..
INCLUDES	= -I.
LIBS	= ./libpthread32.a

##
## MSVC
##
#CC	= cl
#CFLAGS	= /W3 /MT /nologo /Yd /Zi /Fe$@ $^
#BUILD_DIR	= ..
#INCLUDES	= -I.
#LIBS	= pthread.lib

HDR	= pthread.h semaphore.h sched.h
LIB	= libpthread32.a
DLL	= pthread.dll

COPYFILES	= $(HDR) $(LIB) $(DLL)

# If a test case returns a non-zero exit code to the shell, make will
# stop.

TESTS	= mutex1 condvar1 condvar2 exit1 create1 equal1 \
	  exit2 exit3 \
	  join1 mutex2 mutex3 \
	  count1 once1 tsd1 self1 self2 eyal1 \
	  condvar3 condvar4 condvar5 condvar6 \
	  errno1

PASSES	= $(TESTS:%=%.pass)

all:	$(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

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
join1.pass: create1.pass
count1.pass: join1.pass
once1.pass: create1.pass
tsd1.pass: join1.pass
self2.pass: create1.pass
eyal1.pass: tsd1.pass
condvar3.pass: create1.pass
condvar4.pass: create1.pass
condvar5.pass: condvar4.pass
condvar6.pass: condvar5.pass
errno1.pass: mutex3.pass

%.pass: %.exe $(LIB) $(DLL) $(HDR)
	$*
	@ $(ECHO) Passed
	@ $(TOUCH) $@

%.exe: %.c
	@ $(CC) $(CFLAGS) $(INCLUDES) $(LIBS)

$(COPYFILES):
	@ $(ECHO) Copying $@
	@ $(CP) $(BUILD_DIR)\$@ .

clean:
	- $(RM) *.dll
	- $(RM) pthread.h
	- $(RM) semaphore.h
	- $(RM) sched.h
	- $(RM) *.a
	- $(RM) *.exe
	- $(RM) *.pass

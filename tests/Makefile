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
CC	= gcc
CFLAGS	= -g -O2 -UNDEBUG -Wall -o $@ $^
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

HDR	= pthread.h
LIB	= libpthread32.a
DLL	= pthread.dll

# If a test case returns a non-zero exit code to the shell, make will
# stop.

TESTS	= count1 create1 equal1 exit1 exit2 exit3 \
	 join1 mutex1 mutex2 mutex3 \
	 once1 self1 self2 condvar1 condvar2 condvar3 condvar4 tsd1

PASSES	= $(TESTS:%=%.pass)

all:	$(PASSES)
	@ $(ECHO) ALL TESTS PASSED! Congratulations!

%.pass: %.exe $(LIB) $(DLL) $(HDR)
	$*
	@$(ECHO) Passed
	@ $(TOUCH) $@

%.exe: %.c
	@ $(CC) $(CFLAGS) $(INCLUDES) $(LIBS)

$(LIB):
	@ $(ECHO) Copying the library
	@ $(CP) $(BUILD_DIR)\$@ .

$(HDR):
	@ $(ECHO) Copying the header file
	@ $(CP) $(BUILD_DIR)\$@ .

$(DLL):
	@ $(ECHO) Copying the DLL
	@ $(CP) $(BUILD_DIR)\$@ .

clean:
	- $(RM) *.dll
	- $(RM) $(LIB)
	- $(RM) $(HDR)
	- $(RM) *.exe
	- $(RM) *.pass

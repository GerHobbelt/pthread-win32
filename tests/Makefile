# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.


CP	= copy
RM	= erase
TOUCH	= echo Passed >

#
# Mingw32
#
CC	= gcc
CFLAGS	= -g -O2 -UNDEBUG -Wall -o $@ $^
BUILD_DIR	= ../build
RELEASE_DIR	= ../release
INCLUDES	= -I$(BUILD_DIR)/include
LIBS	= $(BUILD_DIR)/lib/libpthread32.a

##
## MSVC
##
#CC	= cl
#CFLAGS	= /W3 /MT /nologo /Yd /Zi /Fe$@ $^
#BUILD_DIR	= ..\build
#RELEASE_DIR	= ..\release
#INCLUDES	= -I$(BUILD_DIR)\include
#LIBS	= $(BUILD_DIR)\lib\pthread.lib

DLL	= pthread.dll

# If a test case returns a non-zero exit code to the shell, make will
# stop.

TESTS	= count1 create1 equal1 exit1 exit2 \
	 exit3 eyal1 mutex1 mutex2 mutex3 \
	 once1 self1 self2 self3 tsd1

PASSES	= $(TESTS:%=%.pass)

all:	$(PASSES)

%.pass: %.exe $(DLL)
	$*
	$(TOUCH) $@

%.exe: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS)

$(DLL):
	$(CP) ..\build\bin\$@ .

clean:
	- $(RM) *.exe
	- $(RM) *.pass


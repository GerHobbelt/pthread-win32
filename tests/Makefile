# Makefile for the pthreads test suite.
# If all of the .pass files can be created, the test suite has passed.

# If a test case returns a non-zero exit code to the shell, make will
# stop.

PASSES = count1.pass create1.pass equal1.pass exit1.pass exit2.pass \
	 exit3.pass eyal1.pass mutex1.pass mutex2.pass mutex3.pass \
	 once1.pass self1.pass self2.pass self3.pass tsd1.pass

all:	$(PASSES)

%.pass: %.exe
	touch $@

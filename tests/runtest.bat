@echo off

REM Usage: runtest cl|gcc testname testarg ...

echo y | erase /s tmp > nul:
rmdir tmp
mkdir tmp
cd tmp

REM Make sure we start with only those files we expect to need
if exist pthread.dll erase pthread.dll > nul:
if exist pthread.h erase pthread.h > nul:
if exist test.h erase test.h > nul:
if exist pthread.lib erase pthread.lib > nul:
if exist libpthread32.a erase libpthread32.a > nul:
copy ..\..\pthread.dll . > nul:
copy ..\..\pthread.h . > nul:
copy ..\test.h . > nul:
copy ..\..\pthread.lib . > nul:
copy ..\..\libpthread32.a . > nul:

REM Compile the test case
REM  produces aout.exe using the compiler given as %1
call ..\c%1.bat %2 > nul:

echo TEST: %2 [%1] > ..\%2.result

REM Run the test case
aout.exe %3 %4 %5 %6 %7 %8 %9 >> ..\%2.result

REM Clean up
erase aout.exe > nul:
if exist pthread.dll erase pthread.dll > nul:
if exist pthread.h erase pthread.h > nul:
if exist pthread.lib erase pthread.lib > nul:
if exist libpthread32.a erase libpthread32.a > nul:
cd ..
more < %2.result


@echo off
cd tmp

REM Make sure we start with only those files we expect to need
if exist pthread.dll erase pthread.dll
if exist pthread.h erase pthread.h
if exist pthread.lib erase pthread.lib
if exist libpthread32.a erase libpthread32.a
copy ..\..\pthread.dll .
copy ..\..\pthread.h .
copy ..\..\pthread.lib .
copy ..\..\libpthread32.a .

REM Compile the test case
REM  produces aout.exe using the compiler given as %1
call ..\c%1.bat %2

echo TEST: %2 [%1] > results.txt

REM Run the test case
aout.exe %3 %4 %5 %6 %7 %8 %9 >> results.txt

REM Clean up
erase aout.exe
if exist pthread.dll erase pthread.dll
if exist pthread.h erase pthread.h
if exist pthread.lib erase pthread.lib
if exist libpthread32.a erase libpthread32.a
cd ..
more < tmp\results.txt

@echo off

REM Usage: runtest cl|gcc testname prerequisit testarg ...

if %3==_ goto noprereq
if NOT EXIST %3.pass goto needprereq

:noprereq
if EXIST %2.fail goto forcetest
if EXIST %2.pass goto bypass

:forcetest
if EXIST %2.fail erase %2.fail

REM Make sure we start with only those files we expect to need
if exist tmp\*.* echo y | erase tmp\*.* > nul:
rmdir tmp
mkdir tmp

copy ..\pthread.dll tmp > nul:
copy ..\pthread.h tmp > nul:
copy ..\semaphore.h tmp > nul:
copy ..\sched.h tmp > nul:
copy test.h tmp > nul:
copy ..\pthread.lib tmp > nul:
copy ..\libpthread32.a tmp > nul:

cd tmp

REM Compile the test case
REM  produces aout.exe using the compiler given as %1
call ..\c%1.bat %2 > ..\%2.%1log

if ERRORLEVEL 1 goto cleanup

REM erase ..\%2.%1log

echo TEST: %2 [%1]

REM Run the test case
if EXIST %2.pass erase %2.pass
if EXIST %2.fail erase %2.fail
if EXIST %2.notrun erase %2.notrun
aout.exe %4 %5 %6 %7 %8 %9

set RESULT=%ERRORLEVEL%

if %RESULT% NEQ 0 echo Failed [%RESULT%] > ..\%2.fail
if %RESULT% EQU 0 echo Passed > ..\%2.pass

:cleanup

cd ..

REM Clean up
if exist tmp\*.* echo y | erase tmp\*.* > nul:

if EXIST %2.fail echo Failed [%RESULT%]
if EXIST %2.pass echo Passed [%RESULT%]

:bypass
goto end

:needprereq
echo Test %2 requires %3 to pass before it can run.
echo No Prereq > ..\%2.notrun
goto end

:end

@echo off

if NOT x%1==x-f goto noforce
if EXIST *.pass echo y | erase *.pass > nul:
if EXIST *.fail echo y | erase *.fail > nul:
if EXIST *.notrun echo y | erase *.notrun > nul:

:noforce
call runtest cl loadfree _
call runtest cl mutex1 _
call runtest cl mutex2 _
call runtest cl exit1 _
call runtest cl condvar1 _
call runtest cl self1 _
call runtest cl condvar2 condvar1
call runtest cl create1 mutex2
call runtest cl mutex3 create1
call runtest cl equal1 create1
call runtest cl exit2 create1
call runtest cl exit3 create1
call runtest cl join0 create1
call runtest cl join1 create1
call runtest cl join2 create1
call runtest cl count1 join1
call runtest cl once1 create1
call runtest cl tsd1 join1
call runtest cl self2 create1
call runtest cl cancel1 self2
call runtest cl cancel2 cancel1
call runtest cl eyal1 tsd1
call runtest cl condvar3 create1
call runtest cl condvar4 create1
call runtest cl condvar5 condvar4
call runtest cl condvar6 condvar5
call runtest cl errno1 mutex3
call runtest cl rwlock1 condvar6
call runtest cl rwlock2 rwlock1
call runtest cl rwlock3 rwlock2
call runtest cl rwlock4 rwlock3
call runtest cl rwlock5 rwlock4
call runtest cl rwlock6 rwlock5
call runtest cl context1 cancel2
call runtest cl cancel3 context1
call runtest cl cancel4 cancel3
call runtest cl cleanup1 cancel4
call runtest cl cleanup2 cleanup1
call runtest cl cleanup3 cleanup2
call runtest cl condvar7 cleanup1
call runtest cl condvar8 condvar7
call runtest cl condvar9 condvar8
call runtest cl exception1 cancel4

if NOT EXIST *.notrun goto skip1
echo The following tests did not run (because prerequisite didn't pass?):
for %%f in (*.notrun) do echo %%f
goto skip2
:skip1
echo All tests ran.
:skip2
if NOT EXIST *.fail goto skip3
echo The following tests failed:
for %%f in (*.fail) do echo %%f
goto skip4
:skip3
echo No tests failed.
:skip4

@echo off

if x%1==x-f echo y | erase *.pass > nul:

call runtest cl mutex1
call runtest cl mutex2
call runtest cl exit1
call runtest cl condvar1
call runtest cl self1
call runtest cl condvar2
call runtest cl create1
call runtest cl mutex3
call runtest cl equal1
call runtest cl exit2
call runtest cl exit3
call runtest cl join1
call runtest cl count1
call runtest cl once1
call runtest cl tsd1
call runtest cl self2
call runtest cl eyal1
call runtest cl condvar3
call runtest cl condvar4
call runtest cl condvar5
call runtest cl condvar6
call runtest cl errno1

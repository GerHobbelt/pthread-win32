cd tmp
cl /W3 /MT /nologo /Yd /Zi -I..\.. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c ..\%1.c
cl /Feaout.exe /Zi %1.obj ..\..\pthread.lib
del %1.obj
copy ..\..\pthread.dll .
aout.exe
del aout.exe
del pthread.dll
cd ..

REM Generate preprocessor output
REM cl /E /W3 /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c ..\%1.c > ..\%1.e

REM Generate object file
cl /W3 /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c ..\%1.c

REM Generate executable
cl /Feaout.exe /Zi %1.obj .\pthread.lib
del %1.obj > nul:

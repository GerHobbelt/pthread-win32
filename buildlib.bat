del *.obj
del pthread_*.dll

if x%1==x goto seh

goto %1

:seh

cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c attr.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c cancel.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c cleanup.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c condvar.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c create.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c dll.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c exit.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c fork.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c global.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c misc.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c mutex.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c private.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c sched.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c signal.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c sync.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c tsd.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c semaphore.c
cl /W3 -DSTDCALL=_stdcall /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c rwlock.c

cl /LD /Zi *.obj /Fepthread_SEH.dll /link /nodefaultlib:libcmt /implib:pthread.lib msvcrt.lib /def:pthread.def

:ceh

cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c attr.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c cancel.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c cleanup.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c condvar.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c create.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c dll.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c exit.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c fork.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c global.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c misc.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c mutex.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c private.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c sched.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c signal.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c sync.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c tsd.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c semaphore.c
cl /W3 /GX /TP -D_cplusplus -DSTDCALL=_cdecl /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -c rwlock.c

cl /LD /Zi *.obj /Fepthread_C++.dll /link /nodefaultlib:libcmt /implib:pthread.lib msvcrt.lib /def:pthread.def

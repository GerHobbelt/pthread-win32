del *.obj
del pthread.dll

set EH=

cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c attr.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c cancel.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c cleanup.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c condvar.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c create.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c dll.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c exit.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c fork.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c global.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c misc.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c mutex.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c private.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c sched.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c signal.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c sync.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c tsd.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c semaphore.c
cl /W3 %EH% /MT /nologo /Yd /Zi -I. -D_WIN32_WINNT=0x400 -DSTDCALL=_stdcall -c rwlock.c

cl /LD %EH% /Zi *.obj /Fepthread.dll /link /nodefaultlib:libcmt /implib:pthread.lib msvcrt.lib /def:pthread.def



/*
 * process.h
 *
 * Temporary stand-in for Win32 process.h
 *
 */

HANDLE _beginthreadex(LPSECURITY_ATTRIBUTES security,
		      DWORD stack,
		      unsigned (* start_routine)(void *),
		      LPVOID param,
		      DWORD flags,
		      LPDWORD threadID);

VOID _endthreadex(DWORD);


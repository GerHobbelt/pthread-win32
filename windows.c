/*
 * windows.c
 *
 * This translation unit implements stubs for all of the Windows API
 * calls.  If debugging is on, we dump out diagnostic output for
 * reassurance.
 *
 */

#include <stdio.h>

#include "windows.h"

#define DEBUG 1

#ifdef DEBUG
#define DIAG(fn) fprintf(stderr, "called: %s\n", fn)
#endif /* DEBUG */

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES security,
		   BOOL manualReset,
		   BOOL initialState,
		   LPCTSTR name)
{
	DIAG("CreateEvent");
	return 0;
}

BOOL SetEvent(HANDLE event)
{
	DIAG("SetEvent");
	return TRUE;
}

BOOL ResetEvent(HANDLE event)
{
	DIAG("ResetEvent");
	return TRUE;
}

VOID EnterCriticalSection(LPCRITICAL_SECTION criticalSection)
{
	DIAG("EnterCriticalSection");
}

VOID LeaveCriticalSection(LPCRITICAL_SECTION criticalSection)
{
	DIAG("LeaveCriticalSection");
}

VOID DeleteCriticalSection(LPCRITICAL_SECTION criticalSection)
{
	DIAG("DeleteCriticalSection");
}

VOID InitializeCriticalSection(LPCRITICAL_SECTION criticalSection)
{
	DIAG("InitializeCriticalSection");
}

BOOL TryEnterCriticalSection(LPCRITICAL_SECTION criticalSection)
{
	DIAG("TryEnterCriticalSection");
	return TRUE;
}

DWORD WaitForMultipleObjects(DWORD numObjects,
			     CONST HANDLE * objectArray,
			     BOOL waitForAll,
			     DWORD timeout)
{
	DIAG("WaitForMultipleObjects");
	return 0;
}

DWORD WaitForSingleObject(HANDLE object,
			  DWORD timeout)
{
	DIAG("WaitForSingleObject");
	return 0;
}

DWORD TlsAlloc()
{
	DIAG("TlsAlloc");
	return 0;
}

BOOL TlsFree(DWORD index)
{
	DIAG("TlsFree");
	return TRUE;
}

BOOL TlsSetValue(DWORD index, LPVOID value)
{
	DIAG("TlsSetValue");
	return TRUE;
}

LPVOID TlsGetValue(DWORD index)
{
	DIAG("TlsGetValue");
	return 0;
}

BOOL SetThreadPriority(HANDLE threadHandle, int priority)
{
	DIAG("SetThreadPriority");
	return TRUE;
}

int GetThreadPriority(HANDLE threadHandle)
{
	DIAG("GetThreadPriority");
	return 0;
}

HANDLE _beginthreadex(LPSECURITY_ATTRIBUTES security,
		      DWORD stack,
		      unsigned (* start_routine)(void *),
		      LPVOID param,
		      DWORD flags,
		      LPDWORD threadID)
{
	DIAG("_beginthreadex");
	return 0; 
}

VOID _endthreadex(DWORD thread)
{
	DIAG("_endthreadex");
}

DWORD GetVersion(VOID)
{
	DIAG("GetVersion");
	return 0;
}

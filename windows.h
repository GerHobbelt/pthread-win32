/*
 * windows.h
 *
 * NOT THE REAL windows.h. We're not necessarily concerned
 * that value are correct. Just that the types are defined.
 *
 */

#define WINAPI

#define CONST const

#define DLL_THREAD_ATTACH 0

#define DLL_THREAD_DETACH 1

#define DLL_PROCESS_ATTACH 2

#define DLL_PROCESS_DETACH 3

/* Error numbers */

enum {
  EINVAL,
  ENOMEM,
  ENOSYS,
  EAGAIN
};

typedef void VOID;

typedef int BOOL;

typedef unsigned long DWORD;

typedef unsigned long ULONG;

typedef void * LPVOID;

typedef DWORD * LPDWORD;

typedef char * LPCTSTR;

typedef unsigned long HANDLE;

typedef HANDLE HINSTANCE;

typedef void * LPSECURITY_ATTRIBUTES;

typedef int CRITICAL_SECTION;

typedef CRITICAL_SECTION * LPCRITICAL_SECTION;

HANDLE CreateEvent(LPSECURITY_ATTRIBUTES security,
		   BOOL manualReset,
		   BOOL initialState,
		   LPCTSTR name);

BOOL SetEvent(HANDLE event);

BOOL ResetEvent(HANDLE event);

VOID EnterCriticalSection(LPCRITICAL_SECTION criticalSection);

VOID LeaveCriticalSection(LPCRITICAL_SECTION criticalSection);

VOID DeleteCriticalSection(LPCRITICAL_SECTION criticalSection);

VOID InitializeCriticalSection(LPCRITICAL_SECTION criticalSection);

VOID TryEnterCriticalSection(LPCRITICAL_SECTION criticalSection);

DWORD WaitForMultipleObjects(DWORD numObjects,
			     CONST HANDLE * objectArray,
			     BOOL waitForAll,
			     DWORD timeout);

DWORD WaitForSingleObject(HANDLE object,
			  DWORD timeout);

DWORD TlsAlloc();

BOOL TlsFree(DWORD index);

BOOL TlsSetValue(DWORD index, LPVOID value);

BOOL TlsSetValue(DWORD index, LPVOID value);

BOOL SetThreadPriority(HANDLE threadHandle, int priority);

int GetThreadPriority(HANDLE threadHandle);

HANDLE _beginthreadex(LPSECURITY_ATTRIBUTES security,
		      DWORD stack,
		      unsigned (* start_routine)(void *),
		      LPVOID param,
		      DWORD flags,
		      LPDWORD threadID);

VOID _endthreadex(DWORD);

DWORD GetVersion(VOID);


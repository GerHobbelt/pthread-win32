/*
 * File: fiber1.c
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 *
 *      Contact Email: rpj@callisto.canberra.edu.au
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * --------------------------------------------------------------------------
 *
 * Test
 * -	Do we have fibers?
 *
 * Test Method (Validation or Falsification):
 *
 * Requirements Tested:
 *
 * Features Tested:
 *
 * Cases Tested:
 *
 * Description:
 *
 * Environment:
 *
 * Input:
 *
 * Output:
 *
 * Assumptions:
 *
 * Pass Criteria:
 *
 * Fail Criteria:
 */

#include "test.h"
#include <windows.h>

VOID
__stdcall
CancelFiberFunc(LPVOID lpParameter);

void DisplayFiberInfo(void);

typedef struct
{
   DWORD dwParameter;          // DWORD parameter to fiber (unused)
   DWORD dwFiberResultCode;    // GetLastError() result code
} FIBERDATASTRUCT, *PFIBERDATASTRUCT, *LPFIBERDATASTRUCT;

enum {
  RTN_OK	= 0,
  RTN_USAGE = 1,
  RTN_ERROR = 13
};

#define FIBER_COUNT 2       // max fibers (including primary)

enum {
  PRIMARY_FIBER, // array index to primary fiber
  CANCEL_FIBER   // array index to cancel fiber
};

LPVOID g_lpFiber[FIBER_COUNT];

int main(int argc, char *argv[])
{
   LPFIBERDATASTRUCT fs;

   //
   // Allocate storage for our fiber data structures
   //
   fs = (LPFIBERDATASTRUCT) HeapAlloc(
                              GetProcessHeap(), 0,
                              sizeof(FIBERDATASTRUCT) * FIBER_COUNT);

   if (fs == NULL)
   {
      printf("HeapAlloc error (%d)\n", GetLastError());
      return RTN_ERROR;
   }

   //
   // Convert thread to a fiber, to allow scheduling other fibers
   //
   g_lpFiber[PRIMARY_FIBER]=ConvertThreadToFiber(&fs[PRIMARY_FIBER]);

   if (g_lpFiber[PRIMARY_FIBER] == NULL)
   {
      printf("ConvertThreadToFiber error (%d)\n", GetLastError());
      return RTN_ERROR;
   }

   //
   // Initialize the primary fiber data structure.  We don't use
   // the primary fiber data structure for anything in this sample.
   //
   fs[PRIMARY_FIBER].dwParameter = 0;
   fs[PRIMARY_FIBER].dwFiberResultCode = 0;

   //
   // Create the Cancellation Handler fiber
   //
   g_lpFiber[CANCEL_FIBER]=CreateFiber(0, CancelFiberFunc, &fs[CANCEL_FIBER]);

   if (g_lpFiber[CANCEL_FIBER] == NULL)
   {
      printf("CreateFiber error (%d)\n", GetLastError());
      return RTN_ERROR;
   }

   fs[CANCEL_FIBER].dwParameter = 0x12345678;

   //
   // Switch to the read fiber
   //
   SwitchToFiber(g_lpFiber[CANCEL_FIBER]);

   //
   // We have been scheduled again. Display results from the
   // read/write fibers
   //
   printf("CancelFiber: result code is %lx\n", fs[CANCEL_FIBER].dwFiberResultCode);

   //
   // Delete the fiber
   //
   DeleteFiber(g_lpFiber[CANCEL_FIBER]);

   //
   // Free allocated memory
   //
   HeapFree(GetProcessHeap(), 0, fs);

   return RTN_OK;
}

VOID
__stdcall
CancelFiberFunc(
             LPVOID lpParameter
             )
{
  LPFIBERDATASTRUCT fds = (LPFIBERDATASTRUCT)lpParameter;

  //
  // If this fiber was passed NULL for fiber data, just return,
  // causing the current thread to exit
  //
  if (fds == NULL)
    {
	  printf("Passed NULL fiber data; exiting current thread.\n");
	  return;
    }

  //
  // Update the fiber result code
  //
  fds->dwFiberResultCode = 0xC00EE;

  //
  // Switch back to the primary fiber
  //
  SwitchToFiber(g_lpFiber[PRIMARY_FIBER]);
}

/*
 *
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * --------------------------------------------------------------------------
 * From: Todd Owen <towen@lucidcalm.dropbear.id.au>
 * To: pthreads-win32@sourceware.cygnus.com
 * Subject: invalid page fault when using LoadLibrary/FreeLibrary
 * 
 * hi,
 * for me, pthread.dll consistently causes an "invalid page fault in
 * kernel32.dll" when I load it "explicitly"...to be precise, loading (with
 * LoadLibrary) isn't a problem, it gives the error when I call FreeLibrary.
 * I guess that the dll's cleanup must be causing the error.
 * 
 * Implicit linkage of the dll has never given me this problem.  Here's a
 * program (console application) that gives me the error.
 * 
 * I compile with: mingw32 (gcc-2.95 release), with the MSVCRT add-on (not
 * that the compiler should make much difference in this case).
 * PTHREAD.DLL: is the precompiled 1999-11-02 one (I tried an older one as
 * well, with the same result).
 * 
 * Fascinatingly, if you have your own dll (mycode.dll) which implicitly
 * loads pthread.dll, and then do LoadLibrary/FreeLibrary on _this_ dll, the
 * same thing happens.
 * 
 */

#include "test.h"

int main() {
  HINSTANCE hinst;

  assert((hinst = LoadLibrary("pthread")) != (HINSTANCE) 0);

  Sleep(100);

  FreeLibrary(hinst);
  return 0;
}


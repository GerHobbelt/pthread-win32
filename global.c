/*
 * global.c
 *
 * Description:
 * This translation unit instantiates data associated with the implementation
 * as a whole.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include "pthread.h"
#include "implement.h"


int ptw32_processInitialized = FALSE;
pthread_key_t ptw32_selfThreadKey = NULL;
pthread_key_t ptw32_cleanupKey = NULL;

int ptw32_concurrency = 0;

/* 
 * Function pointer to InterlockedCompareExchange if it exists; otherwise NULL 
 */
PTW32_INTERLOCKED_LONG
(WINAPI *ptw32_interlocked_compare_exchange)(PTW32_INTERLOCKED_LPLONG,
                                             PTW32_INTERLOCKED_LONG,
                                             PTW32_INTERLOCKED_LONG) = NULL;

/*
 * Global lock for testing internal state of PTHREAD_MUTEX_INITIALIZER
 * created mutexes.
 */
CRITICAL_SECTION ptw32_mutex_test_init_lock;

/*
 * Global lock for testing internal state of PTHREAD_COND_INITIALIZER
 * created condition variables.
 */
CRITICAL_SECTION ptw32_cond_test_init_lock;

/*
 * Global lock for testing internal state of PTHREAD_RWLOCK_INITIALIZER
 * created read/write locks.
 */
CRITICAL_SECTION ptw32_rwlock_test_init_lock;

/*
 * Global lock for testing internal state of PTHREAD_SPINLOCK_INITIALIZER
 * created spin locks.
 */
CRITICAL_SECTION ptw32_spinlock_test_init_lock;

#ifdef _UWIN
/*
 * Keep a count of the number of threads.
 */
int pthread_count = 0;
#endif

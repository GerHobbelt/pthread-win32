/*
 * Test for pthread_exit().
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2002 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@ise.canberra.edu.au
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
 * Depends on API functions: pthread_create().
 */

#include "test.h"

#ifdef __CLEANUP_CXX

#define USE_PTHREAD_EXIT
static const int init_counter_value = 3;
static void *ret_value = reinterpret_cast<void *>(1);
static int counter = init_counter_value;

class Guard
{
    const char * const _str;
    int &_ref;
    Guard &operator=(const Guard&);
    Guard(const Guard&);
public:
    Guard(const char * const str, int &ref) : _str(str), _ref(ref) {
        printf("Construct %s [%d->%d]\n", _str, _ref, _ref+++1);
    };
    ~Guard() {
        printf("~Destruct %s [%d->%d]\n", _str, _ref, _ref---1);
    };
};


void *
func(void * arg)
{
    Guard g("func", counter);

#ifdef USE_PTHREAD_EXIT

    pthread_exit(arg);
    assert(0); //Never reached with pthread_exit

#endif //USE_PTHREAD_EXIT

    return ret_value;
}


#endif /*__CLEANUP_CXX */

int
main()
{
#ifndef __CLEANUP_CXX

    printf("Test requires C++ cleanup enabled. Skipped.\n");

#else

    {
        void *ret = 0;
        Guard g("main", counter);
        pthread_t id;
        assert(0 == pthread_create(&id, 0, func, ret_value));
        assert(0 == pthread_join(id, &ret));
        assert(ret == ret_value);
    }

    assert(counter == init_counter_value);

#endif /*__CLEANUP_CXX */

    return 0;
}

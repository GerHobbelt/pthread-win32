/*
 * implement.h
 *
 * Implementation specific (non API) stuff.
 */

#ifndef _IMPLEMENT_H
#define _IMPLEMENT_H

#define _PTHREAD_HASH_INDEX(x) (((ULONG) x) % PTHREAD_THREADS_MAX)

#define _PTHREAD_CANCEL_DEFAULTS \
             (PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_DEFERRED)

/* Handler execution flags. */
#define _PTHREAD_HANDLER_NOEXECUTE 0
#define _PTHREAD_HANDLER_EXECUTE   1

/* Handler popping schemes. */
enum { _PTHREAD_HANDLER_POP_LIFO, _PTHREAD_HANDLER_POP_FIFO };

/* Special value to mark attribute objects as valid. */
#define _PTHREAD_ATTR_INVALID 0xC0FFEE

/* Round a sizeof(type) up to a multiple of sizeof(DWORD).
   This is all compile time arithmetic.
 */
#define RND_SIZEOF(T) (((sizeof(T) / sizeof(DWORD)) + 1) * sizeof(DWORD))

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Primitives to manage threads table entries. */

int _pthread_new_thread_entry(pthread_t thread, 
			      _pthread_threads_thread_t ** entry);

_pthread_threads_thread ** _pthread_find_thread_entry(pthread_t thread);

void _pthread_delete_thread_entry(_pthread_threads_thread_t ** this);

/* Thread cleanup. */

void _pthread_vacuum(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _IMPLEMENT_H */



/*
 * -------------------------------------------------------------
 *
 * $Header$
 *
 * Module: semaphore.h
 *
 * Purpose:
 *      Semaphores aren't actually part of the PThreads standard.
 *      They are defined by the POSIX Standard:
 *
 *              POSIX 1003.1b-1993      (POSIX.1b)
 *
 *      They are supposed to follow the older UNIX convention for
 *      reporting errors. That is, on failure they are supposed
 *      to return a value of -1 and store the appropriate error
 *      number into 'errno'.
 *      HOWEVER,errno cannot be modified in a multithreaded
 *      program on WIN32; therefore, the value is returned as
 *      the function value.
 *      It is recommended that you compare for zero (0) for success
 *      instead of -1 for failure when checking the status of
 *      these functions.
 *
 * Disclaimer:
 *      This software is provided "as is".
 *
 *      The author makes no warranty or representation, either
 *      express or implied, with respect to this software, its
 *      quality, performance, merchantability, or fitness for a
 *      particular purpose. In no event will the author be
 *      liable for direct, indirect, special, incidental, or
 *      consequential damages arising out of the use or inability
 *      to use the software.
 *
 * -------------------------------------------------------------
 */
#if !defined( SEMAPHORE_H )
#define SEMAPHORE_H

#include <process.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

typedef HANDLE sem_t;

int sem_init (sem_t * sem, int pshared, unsigned int value);

int sem_destroy (sem_t * sem);

int sem_trywait (sem_t * sem);

int sem_wait (sem_t * sem);

int sem_post (sem_t * sem);

#ifdef __cplusplus
}				/* End of extern "C" */
#endif				/* __cplusplus */

#endif				/* !SEMAPHORE_H */

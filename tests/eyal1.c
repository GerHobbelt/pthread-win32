/* Simple POSIX threads program.
 *
 * Author: Eyal Lebedinsky eyal@eyal.emu.id.au
 * Written: Sep 1998.
 * Version Date: 12 Sep 1998
 *
 * Do we need to lock stdout or is it thread safe?
 *
 * Used:
 *	pthread_t
 *	pthread_attr_t
 *	pthread_create()
 *	pthread_join()
 *	pthread_mutex_t
 *	PTHREAD_MUTEX_INITIALIZER
 *	pthread_mutex_init() [not used now]
 *	pthread_mutex_destroy()
 *	pthread_mutex_lock()
 *	pthread_mutex_trylock()
 *	pthread_mutex_unlock()
 *
 * What this program does is establish a work queue (implemented using
 * four mutexes for each thread). It then schedules work (by storing
 * a number in 'todo') and releases the threads. When the work is done
 * the threads will block. The program then repeats the same thing once
 * more (just to test the logic) and when the work is done it destroyes
 * the threads.
 *
 * The 'work' we do is simply burning CPU cycles in a loop.
 * The 'todo' work queue is trivial - each threads pops one element
 * off it by incrementing it, the poped number is the 'work' to do.
 * When 'todo' reaches the limit (nwork) the queue is considered
 * empty.
 *
 * The number displayed at the end is the amount of work each thread
 * did, so we can see if the load was properly distributed.
 *
 * The program was written to test a threading setup (not seen here)
 * rather than to demonstrate correct usage of the pthread facilities.
 *
 * Note how each thread is given access to a thread control structure
 * (TC) which is used for communicating to/from the main program (e.g.
 * the threads knows its 'id' and also filles in the 'work' done).
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>


struct thread_control {
	int		id;
	pthread_t	thread;		/* thread id */
	pthread_mutex_t	mutex_start;
	pthread_mutex_t	mutex_started;
	pthread_mutex_t	mutex_end;
	pthread_mutex_t	mutex_ended;
	long		work;		/* work done */
	int		stat;		/* pthread_init status */
};
typedef struct thread_control	TC;

static TC		*tcs = NULL;
static int		nthreads = 2;
static int		nwork = 0;
static int		quiet = 0;

static int		todo = -1;

static pthread_mutex_t	mutex_todo = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t	mutex_stdout = PTHREAD_MUTEX_INITIALIZER;

/*static pthread_attr_t    pthread_attr_default;*/


static void
die (int ret)
{
	if (NULL != tcs) {
		free (tcs);
		tcs = NULL;
	}

	if (ret)
		exit (ret);
}


static void
waste_time (int n)
{
	int		i;
	double		f;

	f = rand ();

	for (i = n*100; i > 0; --i) {
		f = sqrt (f) * f + 10000.0;
	}
}

static int
do_work_unit (int who, int n)
{
	int		i;
	static int	nchars = 0;

	if (quiet)
		i = 0;
	else {
/* get lock on stdout
*/
		if (pthread_mutex_lock (&mutex_stdout))
			return (-1);

/* do our job
*/
		i = printf ("%c",
			"0123456789abcdefghijklmnopqrstuvwxyz"[who]);
		if (!(++nchars % 50))
			printf ("\n");
		fflush (stdout);

/* release lock on stdout
*/
		if (pthread_mutex_unlock (&mutex_stdout))
			return (-2);
	}

	n = rand () % 10000;	/* ignore incoming 'n' */
	waste_time (n);

	return (n);
}

static int
print_server (void *ptr)
{
	int		mywork;
	int		n;
	TC		*tc = (TC *)ptr;

	if (pthread_mutex_lock (&tc->mutex_started))
		return (-1);

	for (;;) {
		if (pthread_mutex_lock (&tc->mutex_start))
			return (-2);
		if (pthread_mutex_unlock (&tc->mutex_start))
			return (-3);
		if (pthread_mutex_lock (&tc->mutex_ended))
			return (-4);
		if (pthread_mutex_unlock (&tc->mutex_started))
			return (-5);

		for (;;) {

/* get lock on todo list
*/
			if (pthread_mutex_lock (&mutex_todo))
				return (-6);

			mywork = todo;
			if (todo >= 0) {
				++todo;
				if (todo >= nwork)
					todo = -1;
			}
			if (pthread_mutex_unlock (&mutex_todo))
				return (-7);

			if (mywork < 0)
				break;

			if ((n = do_work_unit (tc->id, mywork)) < 0)
				return (-8);
			tc->work += n;
		}

		if (pthread_mutex_lock (&tc->mutex_end))
			return (-9);
		if (pthread_mutex_unlock (&tc->mutex_end))
			return (-10);
		if (pthread_mutex_lock (&tc->mutex_started))
			return (-11);
		if (pthread_mutex_unlock (&tc->mutex_ended))
			return (-12);

		if (-2 == mywork)
			break;
	}

	if (pthread_mutex_unlock (&tc->mutex_started))
		return (-13);

	return (0);
}

static int
dosync (void)
{
	int		i;

	for (i = 0; i < nthreads; ++i) {
		if (pthread_mutex_lock (&tcs[i].mutex_end))
			return (-1);
		if (pthread_mutex_unlock (&tcs[i].mutex_start))
			return (-2);
		if (pthread_mutex_lock (&tcs[i].mutex_started))
			return (-3);
		if (pthread_mutex_unlock (&tcs[i].mutex_started))
			return (-4);
	}

/* Now threads do their work
*/
	for (i = 0; i < nthreads; ++i) {
		if (pthread_mutex_lock (&tcs[i].mutex_start))
			return (-5);
		if (pthread_mutex_unlock (&tcs[i].mutex_end))
			return (-6);
		if (pthread_mutex_lock (&tcs[i].mutex_ended))
			return (-7);
		if (pthread_mutex_unlock (&tcs[i].mutex_ended))
			return (-8);
	}

	return (0);
}

static int
dowork (void)
{
	todo = 0;
	if (dosync () < 0)
		return (-1);

	todo = 0;
	if (dosync () < 0)
		return (-2);

	return (0);
}

int
main (int argc, char *argv[])
{
	int		i;
	int		nargs;

	nthreads  = 1;
	nwork = 100;
	nargs = 0;
	for (i = 1; i < argc; ++i) {
		if (!strcmp ("-q", argv[i])) {
			quiet = 1;
			continue;
		}
		if (!strcmp ("-h", argv[i])) {
			printf ("usage: pthreads [nthreads] [nwork] [-q]\n");
			exit (0);
		}
		switch (++nargs) {
		case 1:
			nthreads  = atoi (argv[i]);
			if (nthreads > 36) {
				printf ("max 36 threads allowed\n");
				die (1);
			}
			break;
		case 2:
			nwork = atoi (argv[i]);
			break;
		default:
			printf ("bad argument '%s'\n", argv[i]);
			die (1);
			break;
		}
	}

	if (NULL == (tcs = calloc (nthreads, sizeof (*tcs))))
		die (1);

/* Launch threads
*/
	for (i = 0; i < nthreads; ++i) {
		tcs[i].id = i;
		pthread_mutex_init (&tcs[i].mutex_start, NULL);
		pthread_mutex_init (&tcs[i].mutex_started, NULL);
		pthread_mutex_init (&tcs[i].mutex_end, NULL);
		pthread_mutex_init (&tcs[i].mutex_ended, NULL);
		tcs[i].work = 0;
		if (pthread_mutex_lock (&tcs[i].mutex_start))
			{}
		tcs[i].stat = pthread_create (&tcs[i].thread,
			NULL /*&pthread_attr_default*/,
			(void*)&print_server, (void *)&tcs[i]);

/* Wait for thread initialisation
*/
		while (!pthread_mutex_trylock (&tcs[i].mutex_started))
			pthread_mutex_unlock (&tcs[i].mutex_started);
	}

	dowork ();

/* Terminate threads
*/
	todo = -2;	/* please terminate */
	if (dosync () < 0)
		die (2);

	for (i = 0; i < nthreads; ++i) {
		if (0 == tcs[i].stat)
			pthread_join (tcs[i].thread, NULL);
	}

/* destroy locks
*/
	pthread_mutex_destroy (&mutex_stdout);
	pthread_mutex_destroy (&mutex_todo);

/* Cleanup
*/
	printf ("\n");

/* Show results
*/
	for (i = 0; i < nthreads; ++i) {
		printf ("%2d ", i);
		if (0 == tcs[i].stat)
			printf ("%10ld\n", tcs[i].work);
		else
			printf ("failed %d\n", tcs[i].stat);
		pthread_mutex_destroy (&tcs[i].mutex_start);
		pthread_mutex_destroy (&tcs[i].mutex_started);
		pthread_mutex_destroy (&tcs[i].mutex_end);
		pthread_mutex_destroy (&tcs[i].mutex_ended);
	}

	die (0);

	return (0);
}

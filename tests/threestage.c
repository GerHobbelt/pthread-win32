/* Chapter 10. ThreeStage.c       Pthreads version              */
/* Three-stage Producer Consumer system                         */

/*  Usage: ThreeStage npc goal                                  */
/* start up "npc" paired producer  and consumer threads.        */
/* Each producer must produce a total of                        */
/* "goal" messages, where each message is tagged                */
/* with the consumer that should receive it                     */
/* Messages are sent to a "transmitter thread" which performs   */
/* additional processing before sending message groups to the   */
/* "receiver thread." Finally, the receiver thread sends        */
/* the messages to the consumer thread.                         */

/* Transmitter: Receive messages one at a time from producers,  */
/* create a transmission message of up to "TRANS_BLOCK" messages*/
/* to be sent to the Receiver. (this could be a network xfer    */
/* Receiver: Take message blocks sent by the Transmitter        */
/* and send the individual messages to the designated consumer  */

#include "test.h"
#include <sys/timeb.h>

#include <pthread.h>
//#include <pthread_exception.h>
#include <stdio.h>
#include <stdlib.h>
#define DELAY_COUNT 1000
#define MAX_THREADS 1024
#define DATA_SIZE 256

#ifndef _WINDOWS
#include <unistd.h>
#include <errno.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* From PWPT (Programming with POSIX Threads, by David Butenhof,
 * Addison-Wesley, 1997) pp 33-34.
 * Define a macro that can be used for diagnostic output from
 * examples. When compiled -DDEBUG, it results in calling printf
 * with the specified argument list. When DEBUG is not defined, it
 * expands to nothing.
 */
#ifdef DEBUG
#define DPRINTF(arg) printf arg
#else
#define DPRINTF(arg)
#endif

/*
 * NOTE: the "do {" ... "} while(o);" bracketing around the macros
 * allows the err_abort and errno_abort macros to be used as if they
 * were function calls, even in contexts where a trailing ";" would
 * generate a null statement. For example,
 *
 *      if (status != 0)
 *              err_abort (status, "message");
 *      else
 *              return status;
 *
 * will not compile if err_abort is a macro ending with "}", because
 * C does not expect a ";" to follow the "}". Because C does expect
 * a ";" following the "}" in the do...while constuct, err_abort and
 * errno_abort can be used as if they were function calls.
 */

#ifdef _WINDOWS
#define errno_get (FormatMessage ( \
    FORMAT_MESSAGE_FROM_SYSTEM, NULL, \
    GetLastError(), MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT), \
    Msg, 256, NULL),Msg)

#define err_abort(code,text) do {\
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, strerror (code)); \
        abort (); \
} while (0)
#define errno_abort(text) do {char Msg[256]; \
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, errno_get); \
        abort (); \
} while (0)
#define err_stop(text) do {char Msg[256]; \
    fprintf (stderr, "%s at %s: %d. %s\n", \
        text, __FILE__, __LINE__, errno_get); \
        return 1; \
} while (0)
#define err_stop0(text) do {\
    fprintf (stderr, "%s at %s: %d.\n", \
        text, __FILE__, __LINE__); \
        return ; \
} while (0)
#else
#define errno_get strerror(errno)
#define err_abort(code,text) do {\
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, strerror (code)); \
        abort (); \
} while (0)
#define errno_abort(text) do { \
    fprintf (stderr, "%s at \"%s\":%d: %s\n", \
        text, __FILE__, __LINE__, errno_get); \
        abort (); \
} while (0)
#define err_stop(text) do { \
    fprintf (stderr, "%s at %s: %d. %s\n", \
        text, __FILE__, __LINE__, errno_get); \
        return 1; \
} while (0)
#define err_stop0(text) do {\
    fprintf (stderr, "%s at %s: %d.\n", \
        text, __FILE__, __LINE__); \
        return ; \
} while (0)
#endif


/*
 * Define a macro that delays for an amount of time proportional
 * to the integer parameter. The delay is a CPU delay and does not
 * voluntarily yield the processor. This simulates computation.
 */

#define delay_cpu(n)  {\
    int i=0, j=0;\
    /* Do some wasteful computations that will not be optimized to nothing */\
    while (i < n) {\
        j = (long)(i*i + (float)(2*j)/(float)(i+1));\
        i++;\
    }\
}

/* Queue lengths and blocking factors. These numbers are arbitrary and  */
/* can be adjusted for performance tuning. The current values are       */
/* not well balanced.                                                   */

#define TRANS_BLOCK 5   /* Transmitter combines this many messages at at time */
#define P2T_QLEN 10     /* Producer to Transmitter queue length */
#define T2R_QLEN 4      /* Transmitter to Receiver queue length */
#define R2C_QLEN 4      /* Receiver to Consumer queue length - there is one such queue for each consumer */

void *
producer(void *);
void *
consumer(void *);
void *
transmitter(void *);
void *
receiver(void *);

typedef struct _THARG
{
  volatile int thread_number;
  volatile int work_goal;         /* used by producers */
  volatile int work_done;         /* Used by producers and consumers */
  char waste[8];                  /* Assure there is no quadword overlap */
} THARG;

/* Basic message as generated by producer and received by consumer */
typedef struct msg_block_tag
{ /* Message block */
  volatile unsigned int checksum; /* Message contents checksum    */
  volatile int source;            /* Creating producer identity      */
  volatile int destination;       /* Identity of receiving thread*/
  volatile int sequence;          /* Message block sequence number */
  time_t timestamp;
  int data[DATA_SIZE];            /* Message Contents                */
} msg_block_t, *pmsg_block_t;

/* Grouped messages sent by the transmitter to receiver         */
typedef struct t2r_msg_tag
{
  volatile int num_msgs;          /* Number of messages contained  */
  msg_block_t messages[TRANS_BLOCK];
} t2r_msg_t;

/* Definitions of a general bounded queue structure. Queues are */
/* implemented as arrays with indices to youngest and oldest    */
/* messages, with wrap around.                                  */
/* Each queue also contains a guard mutex and                   */
/* "not empty" and "not full" condition variables.              */
/* Finally, there is a pointer to an array of messages of       */
/* arbitrary type                                               */

typedef struct queue_tag
{ /* General purpose queue        */
  pthread_mutex_t q_guard;        /* Guard the message block      */
  pthread_cond_t q_ne;            /* Queue is not empty           */
  pthread_cond_t q_nf;            /* Queue is not full            */
  volatile int q_size;            /* Queue max size size          */
  volatile int q_first;           /* Index of oldest message      */
  volatile int q_last;            /* Index of youngest msg        */
  volatile int q_destroyed;       /* Q receiver has terminated   */
  void *msg_array;                /* array of q_size messages     */
} queue_t;
/* Queue management functions */
int
q_initialize(queue_t *, int, int, char *);
int
q_destroy(queue_t *);
int
q_destroyed(queue_t *);
int
q_empty(queue_t *);
int
q_full(queue_t *);
int
q_remove(queue_t *, void *, int);
int
q_insert(queue_t *, void *, int);
void
message_fill(msg_block_t *, int, int, int);
void
message_display(msg_block_t *);

queue_t p2tq, t2rq, *r2cq_array;

static volatile int ShutDown = 0;

int
main(int argc, char * argv[])
{
  int tstatus, nthread, ithread, goal;
  pthread_t *producer_th, *consumer_th, transmitter_th, receiver_th;
  THARG *producer_arg, *consumer_arg;
  char obj_name[32], suffix[10];

  if (argc < 3)
    {
      printf("Usage: ThreeStage npc goal \n");
      return 1;
    }
  srand((int) time(NULL )); /* Seed the RN generator */

  nthread = atoi(argv[1]);
  if (nthread > MAX_THREADS)
    {
      printf("Maximum number of producers or consumers is %d.\n", MAX_THREADS);
      return 2;
    }
  goal = atoi(argv[2]);
  producer_th = malloc(nthread * sizeof(pthread_t));
  producer_arg = calloc(nthread, sizeof(THARG));
  consumer_th = malloc(nthread * sizeof(pthread_t));
  consumer_arg = calloc(nthread, sizeof(THARG));

  if (producer_th == NULL || producer_arg == NULL || consumer_th == NULL || consumer_arg == NULL)
    errno_abort("Cannot allocate working memory for threads.");

  q_initialize(&p2tq, sizeof(msg_block_t), P2T_QLEN, "P2TQ_");
  q_initialize(&t2rq, sizeof(t2r_msg_t), T2R_QLEN, "T2RQ_");
  /* Allocate and initialize Receiver to Consumer queue for each consumer */
  r2cq_array = calloc(nthread, sizeof(queue_t));
  if (r2cq_array == NULL) errno_abort ("Cannot allocate memory for r2c queues");

  for (ithread = 0; ithread < nthread; ithread++)
    {
      /* Initialize r2c queue for this consumer thread */
      strcpy (obj_name, "R2CQ_");
      sprintf (suffix, "%d_", ithread);
      strcat (obj_name, suffix);
      q_initialize (&r2cq_array[ithread], sizeof(msg_block_t),
          R2C_QLEN, obj_name);
      /* Fill in the thread arg */
      consumer_arg[ithread].thread_number = ithread;
      consumer_arg[ithread].work_goal = goal;
      consumer_arg[ithread].work_done = 0;

      tstatus = pthread_create (&consumer_th[ithread], NULL,
          consumer, (void *)&consumer_arg[ithread]);
      if (tstatus != 0)
        err_abort (tstatus, "Cannot create consumer thread");

      producer_arg[ithread].thread_number = ithread;
      producer_arg[ithread].work_goal = goal;
      producer_arg[ithread].work_done = 0;
      tstatus = pthread_create (&producer_th[ithread], NULL,
          producer, (void *)&producer_arg[ithread]);
      if (tstatus != 0)
        err_abort (tstatus, "Cannot create producer thread");
    }

  tstatus = pthread_create(&transmitter_th, NULL, transmitter, NULL );
  if (tstatus != 0)
    err_abort(tstatus, "Cannot create tranmitter thread");
  tstatus = pthread_create(&receiver_th, NULL, receiver, NULL );
  if (tstatus != 0)
    err_abort(tstatus, "Cannot create receiver thread");

  printf("BOSS: All threads are running\n");
  /* Wait for the producers to complete */
  for (ithread = 0; ithread < nthread; ithread++)
    {
      tstatus = pthread_join(producer_th[ithread], NULL );
      if (tstatus != 0)
        err_abort(tstatus, "Cannot join producer thread");
      printf("BOSS: Producer %d produced %d work units\n", ithread,
          producer_arg[ithread].work_done);
    }
  /* Producers have completed their work. */
  printf("BOSS: All producers have completed their work.\n");

  /* Wait for the consumers to complete */
  for (ithread = 0; ithread < nthread; ithread++)
    {
      tstatus = pthread_join(consumer_th[ithread], NULL );
      if (tstatus != 0)
        err_abort(tstatus, "Cannot join consumer thread");
      printf("BOSS: consumer %d consumed %d work units\n", ithread,
          consumer_arg[ithread].work_done);
    }
  printf("BOSS: All consumers have completed their work.\n");

  ShutDown = 1; /* Also set a shutdown flag */

  /* Cancel, and wait for, the transmitter and receiver */
  tstatus = pthread_cancel(transmitter_th);
  if (tstatus != 0)
    err_abort(tstatus, "Failed canceling transmitter");

  tstatus = pthread_cancel(receiver_th);
  if (tstatus != 0)
    err_abort(tstatus, "Failed canceling receiver");

  tstatus = pthread_join(transmitter_th, NULL );
  if (tstatus != 0)
    err_abort(tstatus, "Failed joining transmitter");

  tstatus = pthread_join(receiver_th, NULL );
  if (tstatus != 0)
    err_abort(tstatus, "Failed joining transmitter");

  free(producer_th);
  free(consumer_th);
  free(producer_arg);
  free(consumer_arg);
  free(r2cq_array);

  printf("System has finished. Shutting down\n");

  return 0;
}

void *
producer(void *arg)
{
  THARG * parg;
  int ithread, tstatus;
  msg_block_t msg;

  parg = (THARG *) arg;
  ithread = parg->thread_number;

  while (parg->work_done < parg->work_goal)
    {
      /* Periodically produce work units until the goal is satisfied */
      /* messages receive a source and destination address which are */
      /* the same in this case but could, in general, be different. */
      delay_cpu(DELAY_COUNT * rand() / RAND_MAX);
      message_fill(&msg, ithread, ithread, parg->work_done);

      /* Wait until the p2t queue is not full */
      tstatus = pthread_mutex_lock(&p2tq.q_guard);
      if (tstatus != 0)
        err_abort(tstatus, "Error locking p2tq");

      while (q_full(&p2tq))
        {
          tstatus = pthread_cond_wait(&p2tq.q_nf, &p2tq.q_guard);
          if (tstatus != 0)
            err_abort(tstatus, "Error waiting on p2tq nf");
        }

      /* put the message in the queue */
      q_insert(&p2tq, &msg, sizeof(msg));

      /* Signal that the queue is not empty */
      tstatus = pthread_cond_signal(&p2tq.q_ne);
      if (tstatus != 0)
        err_abort(tstatus, "Error sinaling p2tq ne");

      tstatus = pthread_mutex_unlock(&p2tq.q_guard);
      if (tstatus != 0)
        err_abort(tstatus, "Error unlocking p2tq");

      parg->work_done++;
    }

  return NULL ;
}

/* Cleanup handler to cancel the transmitter thread */
void
cancel_transmitter(void *arg)
{
  q_destroy(&p2tq); /* No receiver for the p2t Q */
  printf("\n** transmitter shutting down\n");
  return;
}

void *
transmitter(void *arg)
{
  /* Obtain multiple producer messages, combining into a single   */
  /* compound message for the receiver */
  /* The transmitter must be prepared to receive cancellation requests */
  int tout, oldstate;
  t2r_msg_t t2r_msg = { 0 };
  msg_block_t p2t_msg;
  struct timespec timeout;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
  pthread_cleanup_push(cancel_transmitter, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);

  while (!ShutDown)
    {
      /* Get a message from the producer to transmitter queue */
      pthread_mutex_lock(&p2tq.q_guard);
      tout = 0;
      while (q_empty(&p2tq) && tout != ETIMEDOUT)
        {
          PTW32_STRUCT_TIMEB currSysTime;
          const DWORD NANOSEC_PER_MILLISEC = 1000000;

          PTW32_FTIME(&currSysTime);
          timeout.tv_sec = (long)currSysTime.time;
          timeout.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;
          timeout.tv_sec += 2;

          tout = pthread_cond_timedwait(&p2tq.q_ne, &p2tq.q_guard, &timeout);
          if (tout != 0 && tout != ETIMEDOUT)
            err_abort(tout, "Error waiting on p2tq ne");
        }

      if (tout != ETIMEDOUT)
        {
          q_remove(&p2tq, &p2t_msg, sizeof(p2t_msg));
          pthread_cond_signal(&p2tq.q_nf);

          /* Fill the transmitter to receiver buffer */
          memcpy(&t2r_msg.messages[t2r_msg.num_msgs], &p2t_msg,
              sizeof(p2t_msg));
          t2r_msg.num_msgs++;
        }
      pthread_mutex_unlock(&p2tq.q_guard);

      /* Transmit the available messages. */
      if (t2r_msg.num_msgs >= TRANS_BLOCK
          || (t2r_msg.num_msgs > 0 && tout == ETIMEDOUT))
        {
          /* a t2r message is full */
          pthread_mutex_lock(&t2rq.q_guard);
          while (q_full(&t2rq))
            {
              pthread_cond_wait(&t2rq.q_nf, &t2rq.q_guard);
            }
          q_insert(&t2rq, &t2r_msg, sizeof(t2r_msg));

          t2r_msg.num_msgs = 0;
          pthread_cond_signal(&t2rq.q_ne);
          pthread_mutex_unlock(&t2rq.q_guard);
        }

    }
  pthread_cleanup_pop(1);
  return NULL ;
}

/* Cleanup handler to cancel the receiver thread */
void
cancel_receiver(void *arg)
{
  q_destroy(&t2rq); /* No receiver for the t2r Q */
  printf("\n** receiver shutting down\n");
  return;
}

void *
receiver(void *arg)
{
  /* Obtain compound messages from the transmitter and unblock them       */
  /* and transmit to the designated consumer.                             */

  int im, ic, oldstate;
  t2r_msg_t t2r_msg;
  msg_block_t r2c_msg;

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
  pthread_cleanup_push (cancel_receiver, NULL);
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);

  while (!ShutDown)
    {
      pthread_mutex_lock(&t2rq.q_guard);
      while (q_empty(&t2rq))
        {
          pthread_cond_wait(&t2rq.q_ne, &t2rq.q_guard);
        }

      q_remove(&t2rq, &t2r_msg, sizeof(t2r_msg));
      pthread_cond_signal(&t2rq.q_nf);
      pthread_mutex_unlock(&t2rq.q_guard);

      /* Distribute the messages to the proper consumer */
      for (im = 0; im < t2r_msg.num_msgs; im++)
        {
          memcpy(&r2c_msg, &t2r_msg.messages[im], sizeof(r2c_msg));
          ic = r2c_msg.destination; /* Destination consumer */

          pthread_mutex_lock(&r2cq_array[ic].q_guard);

          /* Throw the message away if the consumer has stopped */
          if (!q_destroyed(&r2cq_array[ic]))
            {
              while (q_full(&r2cq_array[ic]))
                {
                  pthread_cond_wait(&r2cq_array[ic].q_nf, &r2cq_array[ic].q_guard);
                }
              q_insert(&r2cq_array[ic], &r2c_msg, sizeof(r2c_msg));
              pthread_cond_signal(&r2cq_array[ic].q_ne);
            }
          pthread_mutex_unlock(&r2cq_array[ic].q_guard);
        }
    }
  pthread_cleanup_pop(1);
  return NULL ;
}

void *
consumer(void *arg)
{
  THARG * carg;
  int ithread, tstatus;
  msg_block_t msg;
  queue_t *pr2cq;

  carg = (THARG *) arg;
  ithread = carg->thread_number;
  pr2cq = &r2cq_array[ithread];

  while (carg->work_done < carg->work_goal)
    {
      /* Receive and display messages */
      /* Wait until the r2c queue is not empty */
      tstatus = pthread_mutex_lock(&pr2cq->q_guard);
      if (tstatus != 0)
        err_abort(tstatus, "Error locking r2cq");

      while (q_empty(pr2cq))
        {
          tstatus = pthread_cond_wait(&pr2cq->q_ne, &pr2cq->q_guard);
          if (tstatus != 0)
            err_abort(tstatus, "Error waiting on r2cq ne");
        }

      /* remove the message from the queue */
      q_remove(pr2cq, &msg, sizeof(msg));

      /* Signal that the queue is not full as we've removed a message */
      tstatus = pthread_cond_signal(&pr2cq->q_nf);
      if (tstatus != 0)
        err_abort(tstatus, "Error signaling r2cq nf");

      tstatus = pthread_mutex_unlock(&pr2cq->q_guard);
      if (tstatus != 0)
        err_abort(tstatus, "Error unlocking r2cq");

      printf("\nMessage received by consumer #: %d", ithread);
      message_display(&msg);

      carg->work_done++;
    }

  /* Destroy the queue once the Q's receiving end terminates */
  q_destroy(pr2cq);
  return NULL ;
}

unsigned int
compute_checksum(void * msg, unsigned int length)
{
  /*
   * Compute an xor checksum on the entire message of "length"
   * integers
   */
  unsigned int i, cs = 0, *pint;

  pint = (unsigned int *) msg;
  for (i = 0; i < length; i++)
    {
      cs = (cs ^ *pint);
      pint++;
    }
  return cs;
}

void
message_fill(msg_block_t *mblock, int src, int dest, int seqno)
{
  /* Fill the message buffer, and include checksum and timestamp  */
  /* This function is called from the producer thread while it    */
  /* owns the message block mutex                                 */

  int i;

  mblock->checksum = 0;
  for (i = 0; i < DATA_SIZE; i++)
    {
      mblock->data[i] = rand();
    }
  mblock->source = src;
  mblock->destination = dest;
  mblock->sequence = seqno;
  mblock->timestamp = time(NULL );
  mblock->checksum = compute_checksum(mblock,
      sizeof(msg_block_t) / sizeof(int));
  /*      printf ("Generated message: %d %d %d %d %x %x\n",
           src, dest, seqno, mblock->timestamp,
           mblock->data[0], mblock->data[DATA_SIZE-1]);  */
  return;
}

void
message_display(msg_block_t *mblock)
{
  /* Display message buffer and timestamp, validate checksum      */
  /* This function is called from the consumer thread while it    */
  /* owns the message block mutex                                 */
  unsigned int tcheck = 0;

  tcheck = compute_checksum(mblock, sizeof(msg_block_t) / sizeof(int));
  printf("\nMessage number %d generated at: %s", mblock->sequence,
      ctime(&(mblock->timestamp)));
  printf("Source and destination: %d %d\n", mblock->source,
      mblock->destination);
  printf("First and last entries: %x %x\n", mblock->data[0],
      mblock->data[DATA_SIZE - 1]);
  if (tcheck == 0 /*mblock->checksum was 0 when CS first computed */)
    printf("GOOD ->Checksum was validated.\n");
  else
    printf("BAD  ->Checksum failed. message was corrupted\n");

  return;

}

/* Finite bounded queue management functions */
int
q_initialize(queue_t *q, int msize, int nmsgs, char *name)
{
  /* Initialize queue, including its mutex and CVs, which are named */
  /* Allocate storage for all messages. */
  char obj_name[32];

  q->q_first = q->q_last = 0;
  q->q_size = nmsgs;
  q->q_destroyed = 0;

  strcpy(obj_name, name);
  strcat(obj_name, "q_guard");
  pthread_mutex_init(&q->q_guard, NULL );
//  pthread_mutex_setname_np(&q->q_guard, obj_name, 0);

  strcpy(obj_name, name);
  strcat(obj_name, "q_ne");
  pthread_cond_init(&q->q_ne, NULL );
//  pthread_cond_setname_np(&q->q_ne, obj_name, 0);

  strcpy(obj_name, name);
  strcat(obj_name, "q_nf");
  pthread_cond_init(&q->q_nf, NULL );
//  pthread_cond_setname_np(&q->q_nf, obj_name, 0);

  if ((q->msg_array = calloc(nmsgs, msize)) == NULL )
    return 1;
  return 0; /* No error */
}

int
q_destroy(queue_t *q)
{
  /* Free all the resources created by q_initialize */
  q->q_destroyed = 1;
  free(q->msg_array);
  pthread_mutex_destroy(&q->q_guard);
  pthread_cond_destroy(&q->q_ne);
  pthread_cond_destroy(&q->q_nf);
  return 0;
}

int
q_destroyed(queue_t *q)
{
  return (q->q_destroyed);
}

int
q_empty(queue_t *q)
{
  return (q->q_first == q->q_last);
}

int
q_full(queue_t *q)
{
  return ((q->q_last - q->q_first) == 1
      || (q->q_first == q->q_size - 1 && q->q_last == 0));
}

int
q_remove(queue_t *q, void * msg, int msize)
{
  char *pm;

  pm = (char *) q->msg_array;
  /* Remove oldest ("first") message */
      memcpy(msg, pm + (q->q_first * msize), msize);
  q->q_first = ((q->q_first + 1) % q->q_size);
  return 0; /* no error */
}

int
q_insert(queue_t *q, void * msg, int msize)
{
  char *pm;

  pm = (char *) q->msg_array;
  /* Add a new youngest ("last") message */
  if (q_full(q))
    return 1; /* Error - Q is full */
  memcpy(pm + (q->q_last * msize), msg, msize);
  q->q_last = ((q->q_last + 1) % q->q_size);

  return 0;
}

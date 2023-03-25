/**
 *  \file fifo.c (implementation file)
 *
 *  \brief Problem name: Producers / Consumers.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Data transfer region implemented as a monitor.
 *
 *  Definition of the operations carried out by the producers / consumers:
 *     \li putVal
 *     \li getVal.
 *
 *  \author António Rui Borges - March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include "probConst.h"
#include "dataStructures.h"

/** \brief return status on monitor initialization */
extern int statusInitMon;

/** \brief producer threads return status array */
extern int statusProd[1];

/** \brief consumer threads return status array */
extern int* statusWorkers;

/** \brief number of storage positions in the data transfer region */
extern int nStorePos;

/** \brief storage region */
static Chunk* mem;

/** \brief insertion pointer */
static unsigned int ii;

/** \brief retrieval pointer */
static unsigned int ri;

/** \brief flag signaling the data transfer region is full */
static bool full;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;;

/** \brief producers synchronization point when the data transfer region is full */
static pthread_cond_t fifoFull;

/** \brief consumers synchronization point when the data transfer region is empty */
static pthread_cond_t fifoEmpty;

static bool lastChunk;



/**
 *  \brief Initialization of the data transfer region.
 *
 *  Internal monitor operation.
 */

static void initialization (void)
{
  if ((( mem = (Chunk*) malloc (nStorePos * sizeof (Chunk))) == NULL))
	 { fprintf (stderr, "error on allocating space to the data transfer region\n");
	   statusInitMon = EXIT_FAILURE;
	   pthread_exit (&statusInitMon);
     }

	                                                                               /* initialize FIFO in empty state */
  ii = ri = 0;                                        /* FIFO insertion and retrieval pointers set to the same value */
  full = false;                                                                                  /* FIFO is not full */
  lastChunk = false;

  pthread_cond_init (&fifoFull, NULL);                                 /* initialize producers synchronization point */
  pthread_cond_init (&fifoEmpty, NULL);                                /* initialize consumers synchronization point */
}

/**
 *  \brief Store a value in the data transfer region.
 *
 *  Operation carried out by the producers.
 *
 *  \param prodId producer identification
 *  \param val value to be stored
 */

void putChunk (unsigned int prodId, Chunk val, bool lt)
{
  if ((statusProd[prodId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusProd[prodId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusProd[prodId] = EXIT_FAILURE;
       pthread_exit (&statusProd[prodId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while (full)                                                           /* wait if the data transfer region is full */
  { if ((statusProd[prodId] = pthread_cond_wait (&fifoFull, &accessCR)) != 0)
       { errno = statusProd[prodId];                                                          /* save error in errno */
         perror ("error on waiting in fifoFull");
         statusProd[prodId] = EXIT_FAILURE;
         pthread_exit (&statusProd[prodId]);
       }
  }
  mem[ii] = val;                                                                          /* store value in the FIFO */
  ii = (ii + 1) % nStorePos;
  full = (ii == ri);
  lastChunk = lt;


  if(lastChunk) {
        if ((statusProd[prodId] = pthread_cond_broadcast (&fifoEmpty)) != 0)      /* let a consumer know that a value has been
                                                                                                               stored */
        { errno = statusProd[prodId];                                                             /* save error in errno */
        perror ("error on signaling in fifoEmpty");
        statusProd[prodId] = EXIT_FAILURE;
        pthread_exit (&statusProd[prodId]);
        }
    } else {
        if ((statusProd[prodId] = pthread_cond_signal (&fifoEmpty)) != 0)      /* let a consumer know that a value has been
                                                                                                               stored */
        { errno = statusProd[prodId];                                                             /* save error in errno */
        perror ("error on signaling in fifoEmpty");
        statusProd[prodId] = EXIT_FAILURE;
        pthread_exit (&statusProd[prodId]);
        }
    }
  

  if ((statusProd[prodId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
     { errno = statusProd[prodId];                                                            /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusProd[prodId] = EXIT_FAILURE;
       pthread_exit (&statusProd[prodId]);
     }
}

/**
 *  \brief Get a value from the data transfer region.
 *
 *  Operation carried out by the consumers.
 *
 *  \param consId consumer identification
 *
 *  \return value
 */

int getChunk (unsigned int consId, Chunk* res)
{      
                                                                         /* retrieved value */
  if ((statusWorkers[consId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
     { errno = statusWorkers[consId];                                                            /* save error in errno */
       perror ("error on entering monitor(CF)");
       statusWorkers[consId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[consId]);
     }
  pthread_once (&init, initialization);                                              /* internal data initialization */

  while (!lastChunk && (ii == ri) && !full)                                           /* wait if the data transfer region is empty */
  { 
    if ((statusWorkers[consId] = pthread_cond_wait (&fifoEmpty, &accessCR)) != 0)
       { errno = statusWorkers[consId];                                                          /* save error in errno */
         perror ("error on waiting in fifoEmpty");
         statusWorkers[consId] = EXIT_FAILURE;
         pthread_exit (&statusWorkers[consId]);
       }
  }
  if(lastChunk && (ii == ri) && !full) { /* If FIFO is empty and last chunk was added exit monitor and return 1*/
    if ((statusWorkers[consId] = pthread_mutex_unlock (&accessCR)) != 0)                                   /* exit monitor */
     { errno = statusWorkers[consId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusWorkers[consId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[consId]);
     }
    return 1;
  }

  *res = mem[ri];                                                                   /* retrieve a  value from the FIFO */
  ri = (ri + 1) % nStorePos;
  full = false;


  if ((statusWorkers[consId] = pthread_cond_signal (&fifoFull)) != 0)       /* let a producer know that a value has been
                                                                                                            retrieved */
     { errno = statusWorkers[consId];                                                             /* save error in errno */
       perror ("error on signaling in fifoFull");
       statusWorkers[consId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[consId]);
     }
  if ((statusWorkers[consId] = pthread_mutex_unlock (&accessCR)) != 0)                                   /* exit monitor */
     { errno = statusWorkers[consId];                                                             /* save error in errno */
       perror ("error on exiting monitor(CF)");
       statusWorkers[consId] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[consId]);
     }
  
  return 0;
}

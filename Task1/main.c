/**
 *  \file producersworkers.c (implementation file)
 *
 *  \brief Problem name: Producers / workers.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author António Rui Borges - March 2023
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "probConst.h"
#include "fifo.h"
#include "countWords.h"
#include "sharedRegion.h"

/** \brief return status on monitor initialization */
int statusInitMon;

/** \brief producer threads return status array */
int statusProd[1];

/** \brief worker threads return status array */
int* statusWorkers;

/** \brief number of storage positions in the data transfer region */
int nStorePos = K;

/** \brief producer life cycle routine */
static void *producer (void *id);

/** \brief worker life cycle routine */
static void *worker (void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

/** \brief print command usage */
static void printUsage (char *cmdName);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (producers and workers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */

int main (int argc, char *argv[])
{
  /* process command line options */

  int opt;                                                                                        /* selected option */
  int nThreads = N;                                                               /* number of threads to be created */

  // do
  // { switch ((opt = getopt (argc, argv, "t:p:h")))
  //   { case 't': /* number of threads to be created */
  //               if (atoi (optarg) <= 0)
  //                  { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
  //                    printUsage (basename (argv[0]));
  //                    return EXIT_FAILURE;
  //                  }
  //               nThreads = (int) atoi (optarg);
  //               if (nThreads > N)
  //                  { fprintf (stderr, "%s: too many threads\n", basename (argv[0]));
  //                    printUsage (basename (argv[0]));
  //                    return EXIT_FAILURE;
  //                  }
  //               break;
  //     case 'p': /* number of storage positions in the data transfer region */
  //               if (atoi (optarg) <= 0)
  //                  { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
  //                    printUsage (basename (argv[0]));
  //                    return EXIT_FAILURE;
  //                  }
  //               nStorePos = (int) atoi (optarg);
  //               if (nStorePos > K)
  //                  { fprintf (stderr, "%s: too many storage positions\n", basename (argv[0]));
  //                    printUsage (basename (argv[0]));
  //                    return EXIT_FAILURE;
  //                  }
  //               break;
  //     case 'h': /* help mode */
  //               printUsage (basename (argv[0]));
  //               return EXIT_SUCCESS;
  //     case '?': /* invalid option */
  //               fprintf (stderr, "%s: invalidxx option\n", basename (argv[0]));
  //   	        printUsage (basename (argv[0]));
  //               return EXIT_FAILURE;
  //     case -1:  break;
  //   }
  // } while (opt != -1);
  // if (optind < argc)
  //    { fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
  //      printUsage (basename (argv[0]));
  //      return EXIT_FAILURE;
  //    }
  if (((statusWorkers = malloc (nThreads * sizeof (int))) == NULL))
     { fprintf (stderr, "error on allocating space to the return status arrays of producer / worker threads\n");
       exit (EXIT_FAILURE);
     }

  pthread_t* tIdWorkers;                                                          /* workers internal thread id array */
  unsigned int* work;                                               /* workers application defined thread id array */
  int i;                                                                                        /* counting variable */
  int *pStatus;                                                                       /* pointer to execution status */

  /* initializing the application defined thread id arrays for the producers and the workers and the random number
     generator */
  if (
      ((tIdWorkers = malloc (nThreads * sizeof (pthread_t))) == NULL) ||
      ( ((work = malloc (nThreads * sizeof (unsigned int))) == NULL)))
     { fprintf (stderr, "error on allocating space to both internal / external producer / worker id arrays\n");
       exit (EXIT_FAILURE);
     }
  for (i = 0; i < nThreads; i++)
    work[i] = i;
  srandom ((unsigned int) getpid ());
  (void) get_delta_time ();

  /* generation of intervening entities threads */

  for (i = 0; i < nThreads; i++)
    if (pthread_create (&tIdWorkers[i], NULL, worker, &work[i]) != 0)                             /* thread worker */
       { perror ("error on creating thread worker");
         exit (EXIT_FAILURE);
       }

  for(int f = 1 ; f < argc ; f++){
    FILE * fp;
    int display;

    if( fopen(argv[f], "r") == NULL ){
        printf("File %s doesn't exist\n", argv[f]);
        continue;
    }
    fp = fopen(argv[f], "r");
    bool endF = false;

    while (1) {
      int textChunk[6000];
      int b = 0;

      for(; b < 4096 ; b++ ){
        // reading file
        display = fgetc(fp);
        textChunk[b] = display;
        // end of file indicator
        if (feof(fp))
        {
          endF = true;
          break;
        } 
        // UTF-8  -->  E280**
        if( display == 226)
        {
          display = fgetc(fp);
          b += 1;
          textChunk[b] = display;
          display = fgetc(fp);
          b += 1;
          textChunk[b] = display;
        }
        else if( display == 195 )
        {
          display = fgetc(fp);
          b += 1;
          textChunk[b] = display;
        }
        // end of file indicator
        if (feof(fp))
        {
          endF = true;
          break;
        } 

      }
      /* Create a data chunk*/
      Chunk *save = (Chunk *)malloc(sizeof(Chunk));
      save->fileID = f;
      save->numBytes = b;
      for(int i = 0 ; i < b ; i++ )
      {
        save->textChunk[i] = textChunk[i];
      }
      if( endF && f == argc - 1) putChunk(0, *save, true );
      else putChunk(0, *save, false );
      //printf("%i\n", textChunk[b-1]);

      if(endF) break;
    }
  }

  /* waiting for the termination of the intervening entities threads */

  printf ("\nFinal report\n");
  for (i = 0; i < nThreads; i++)
  { if (pthread_join (tIdWorkers[i], (void *) &pStatus) != 0)                                       /* thread worker */
       { perror ("error on waiting for thread customer");
         exit (EXIT_FAILURE);
       }
    printf ("thread worker, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *pStatus);
  }
  printf ("\nElapsed time = %.6f s\n", get_delta_time ());

  exit (EXIT_SUCCESS);
}


/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker (void *par)
{
  unsigned int id = *((unsigned int *) par);                    /* worker id */

  Chunk *chunk = (Chunk *)malloc(sizeof(Chunk)); /* Chunk value */
  TempResults *res = (TempResults *)malloc(sizeof(TempResults)); /* Partialresult value */

  while (getChunk(id, chunk) != 1) /* get available data chunks until all chunks are processed */
  {
      count(chunk, res); /* process data chunk */
      savePartialResults(id, res); /* save the partial results */
  }
  statusWorkers[id] = EXIT_SUCCESS;
  pthread_exit (&statusWorkers[id]);
}

/**
 *  \brief Get the process time that has elapsed since last call of this time.
 *
 *  \return process elapsed time
 */

static double get_delta_time(void)
{
  static struct timespec t0, t1;

  t0 = t1;
  if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0)
  {
    perror ("clock_gettime");
    exit(1);
  }
  return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 *  \brief Print command usage.
 *
 *  A message specifying how the program should be called is printed.
 *
 *  \param cmdName string with the name of the command
 */

static void printUsage (char *cmdName)
{
  fprintf (stderr, "\nSynopsis: %s [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -t nThreads  --- set the number of threads to be created (default: 4)\n"
           "  -p nStorePos --- set the number of storage positions in the data transfer region (default: 4)\n"
           "  -h           --- print this help\n", cmdName);
}
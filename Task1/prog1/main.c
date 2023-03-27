/**
 *  \file main.c (implementation file)
 *
 *  \brief Problem name: Text processing in Portuguese
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author João Morais and Miguel Ferreira
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

/** \brief main thread return status array */
int* statusMain;

/** \brief worker threads return status array */
int* statusWorkers;

/** \brief number of storage positions in the data transfer region */
int nStorePos = K;

/** \brief worker life cycle routine */
static void *worker (void *id);

/** \brief execution time measurement */
static double get_delta_time(void);

// /** \brief print command usage */
// static void printUsage (char *cmdName);

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (main and workers) and
 *  waiting for their termination.
 *
 *  \param argc number of words of the command line
 *  \param argv list of words of the command line
 *
 *  \return status of operation
 */

int main (int argc, char *argv[])
{
  int nThreads = N;                                                               /* number of threads to be created */

  if( strcmp(argv[1],"-t") == 0 ) nThreads = *argv[2] - '0';  /* number of threads to be created */


  if (((statusMain = malloc (sizeof (int))) == NULL))
  { 
    fprintf (stderr, "error on allocating space to the return status arrays of producer / worker threads\n");
    exit (EXIT_FAILURE);
  }
  if (((statusWorkers = malloc (nThreads * sizeof (int))) == NULL))
  { 
    fprintf (stderr, "error on allocating space to the return status arrays of producer / worker threads\n");
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

  int starting = 0;
  if ( strcmp(argv[1],"-t") == 0 ) starting = 2;
  char* files[(argc - (starting + 1) )];
  memset(files,0,(argc - (starting + 1) ));
  for(int f = starting + 1 ; f < argc ; f++) files[f-(starting + 1)] = argv[f];
  storeFileNames(0,argc-(starting+1),files);
  for(int f = starting + 1 ; f < argc ; f++)
  {
    FILE * fp;
    int display;

    if( fopen(argv[f], "r") == NULL ){
        printf("File %s doesn't exist\n", argv[f]);
        continue;
    }
    fp = fopen(argv[f], "r");
    bool endF = false;
    
    bool inWord = false;
    int newWord[] = {166,147,157,156,32,46,44,58,59,45,63,33,34,40,41,91,9,10,13,194,187,171};
    int arrLen = sizeof newWord / sizeof newWord[0];
    int prev = 0;
    while (1) {
      int textChunk[5000];
      int b = 0;

      while(1){
        // reading file
        display = fgetc(fp);
        textChunk[b] = display;
        bool special = false;
        bool pastInWord = inWord;
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
          special = true;
        }
        if( prev != 195 )
        {
          bool tmpInWord = true;
          if(!inWord) if ( display == 95 ) tmpInWord = true;
          if(inWord) if ( display == 39 || display == 95 || ( special && display == 152 ) || ( special && display == 153 )) tmpInWord = true;
          if(inWord) for (int i = 0; i < arrLen; i++ ) if (newWord[i] == display || ( special && display == 93 ) || ( special && display == 148 )) tmpInWord = false;
          if(!inWord) for (int i = 0; i < arrLen; i++) if (newWord[i] == display || display == 39 || ( special && display == 93 ) || ( special && display == 152 ) || ( special && display == 153 ) || ( special && display == 148 )) tmpInWord = false;
          inWord = tmpInWord;
          if( pastInWord && !inWord ) if(b >= 4096) break;
        }
        if( display == 195 )
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
        b +=1 ;
        prev = display;
      }
      /* Create a data chunk*/
      Chunk *save = (Chunk *)malloc(sizeof(Chunk));
      save->fileID = f-(starting + 1);
      save->numBytes = b;
      for(int i = 0 ; i < b ; i++ )
      {
        save->textChunk[i] = textChunk[i];
      }
      if( endF && f == argc - 1) putChunk(0, *save, true );
      else putChunk(0, *save, false );
      //printf("%i\n",b);

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
  printProcessingResults(0);
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
  TempResults *res = (TempResults *)malloc(sizeof(TempResults)); /* TempResults value */

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

// /**
//  *  \brief Print command usage.
//  *
//  *  A message specifying how the program should be called is printed.
//  *
//  *  \param cmdName string with the name of the command
//  */

// static void printUsage (char *cmdName)
// {
//   fprintf (stderr, "\nSynopsis: %s [OPTIONS]\n"
//            "  OPTIONS:\n"
//            "  -t nThreads  --- set the number of threads to be created (default: 4)\n", cmdName);
// }
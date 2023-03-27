/**
 *  \file sharedRegion.c (implementation file)
 *
 *  \brief Problem name: Text processing in Portuguese
 *  This module implements and stores information shared by the main and worker thread
 * 
 *
 *  \author João Morais and Miguel Ferreira
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "probConst.h"
#include "dataStructures.h"

/** \brief Number of files to be processed */
int nFiles;

/** \brief Names of files to be processed */
char **fNames;

/** \brief worker threads return status array */
extern int* statusWorkers;

/** \brief producer thread return status array */
extern int* statusMain;

/** \brief chunks storage region */
static TempResults* mem;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

/** \brief flag which warrants that the data transfer region is initialized exactly once */
static pthread_once_t init = PTHREAD_ONCE_INIT;



/**
 *  \brief Initializes the data transfer region.
 *
 *  This function is called exactly once by an worker thread.
 *  It initializes the data transfer region.
 *
 */
void initialization (void)
{
    /* initialize FIFO in empty state */
    mem = (TempResults*) malloc(sizeof(TempResults)*nFiles);
}

/**
 *  \brief Store the names of files to be processed from the shared region structure.
 *
 *  \param threadID thread identification
 *  \param numFiles number of files to be processed
 *  \param fileNames pointer to the array of file names
 *
 */
void storeFileNames(unsigned int threadID, int numFiles, char *fileNames[])
{
    
    if ((statusMain[threadID] = pthread_mutex_lock (&accessCR)) != 0)       /* enter monitor */
     { errno = statusMain[threadID];                                  /* save error in errno */
       perror ("error on entering monitor(CF)");
         statusMain[threadID] = EXIT_FAILURE;
       pthread_exit (&statusMain[threadID]);
     }
     
    nFiles = numFiles; /* Store values on the shared region */
    fNames = fileNames;

    if ((statusMain[threadID] = pthread_mutex_unlock (&accessCR)) != 0)       /* exit monitor */
     { errno = statusMain[threadID];                     /* save error in errno */
       perror ("error on exiting monitor(CF)");
         statusMain[threadID] = EXIT_FAILURE;
       pthread_exit (&statusMain[threadID]);
     }
}

/**
 *  \brief Print the processing results.
 *
 *  \param threadID thread identification
 *
 */
void printProcessingResults(unsigned int threadID)
{
    if ((statusMain[threadID] = pthread_mutex_lock (&accessCR)) != 0)       /* enter monitor */
    { errno = statusMain[threadID];                                  /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusMain[threadID] = EXIT_FAILURE;
        pthread_exit (&statusMain[threadID]);
    }
    for (int i = 0; i < nFiles; ++i) {
        printf("\nFile name: %s:\n", fNames[i]);
        printf("Total number of words = %d\n", mem[i].nWords);
        printf("N. of words witn an\n");
        printf("\tA\tE\tI\tO\tU\tY\tC\n");
        printf("\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", mem[i].a, mem[i].e, mem[i].i, mem[i].o, mem[i].u, mem[i].y, mem[i].c);
    }



    if ((statusMain[threadID] = pthread_mutex_unlock (&accessCR)) != 0)       /* exit monitor */
    { errno = statusMain[threadID];                     /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusMain[threadID] = EXIT_FAILURE;
        pthread_exit (&statusMain[threadID]);
    }
}

/**
 *  \brief Adds the results from each worker
 *
 *  \param threadID thread identification
 *  \param partialResults partial results obtained by processing a chunk 
 */
void savePartialResults(unsigned int threadID, TempResults *partialResults)
{

    if ((statusWorkers[threadID] = pthread_mutex_lock (&accessCR)) != 0)       /* enter monitor */
     { errno = statusWorkers[threadID];                                  /* save error in errno */
       perror ("error on entering monitor(CF)");
         statusWorkers[threadID] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[threadID]);
     }
    pthread_once (&init, initialization);
    mem[partialResults->fileID].nWords += partialResults->nWords;
    mem[partialResults->fileID].a += partialResults->a;
    mem[partialResults->fileID].e += partialResults->e;
    mem[partialResults->fileID].i += partialResults->i;
    mem[partialResults->fileID].o += partialResults->o;
    mem[partialResults->fileID].u += partialResults->u;
    mem[partialResults->fileID].c += partialResults->c;
    mem[partialResults->fileID].y += partialResults->y;

    if ((statusWorkers[threadID] = pthread_mutex_unlock (&accessCR)) != 0)       /* exit monitor */
     { errno = statusWorkers[threadID];                     /* save error in errno */
       perror ("error on exiting monitor(CF)");
         statusWorkers[threadID] = EXIT_FAILURE;
       pthread_exit (&statusWorkers[threadID]);
     }
}


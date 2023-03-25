/**
 *  \file sharedRegion.h (implementation file)
 *
 *  \brief Assignment 1 - problem 1.
 *
 *  This module implements and stores information shared by the main and worker thread
 * 
 *
 *  \author Hugo Leal 93059 & Luísa Amaral 93001
 */
#ifndef SHAREDREGION_H
#define SHAREDREGION_H
#include "dataStructures.h"
/**
 *  \brief Initializes the data transfer region.
 *
 *  This function is called exactly once by an worker thread.
 *  It initializes the data transfer region.
 *
 */
void initialization (void);

/**
 *  \brief Store the names of files to be processed from the shared region structure.
 *
 *  Operation carried out by the main thread when obtaining the filenames from the command line.
 *
 *  \param threadID thread identification
 *  \param numFiles number of files to be processed
 *  \param fileNames pointer to the array of file names
 *
 */
void storeFileNames(unsigned int threadID, int numFiles, char *fileNames[]);

/**
 *  \brief Print the processing results.
 *
 *  Operation carried out by the main thread after all worker threads are finished processing chunks.
 *
 *  \param threadID thread identification
 *
 */
void printProcessingResults(unsigned int threadID);

/**
 *  \brief Appends the partial results of a worker processing chunk to the shared region.
 *
 *  Operation carried out by the worker thread after processing chunk.
 *
 *  \param threadID thread identification
 *  \param TempResults partial results obtained by processing a chunk 
 */
void savePartialResults(unsigned int threadID, TempResults *partialResults);

#endif /* SHAREDREGION_H */
/**
 *  \file sharedRegion.h 
 *  \brief Problem name: Text processing in Portuguese
 * 
 *
 *  \author João Morais and Miguel Ferreira
 */
#ifndef SHAREDREGION_H
#define SHAREDREGION_H
#include "dataStructures.h"
/**
 *  \brief Initializes the data transfer region.
 *
 */
void initialization (void);

/**
 *  \brief Store the names of files to be processed from the shared region structure.
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
 *  \param threadID thread identification
 *
 */
void printProcessingResults(unsigned int threadID);

/**
 *  \brief Adds the results from each worker
 *
 *  \param threadID thread identification
 *  \param partialResults partial results obtained by processing a chunk 
 */
void savePartialResults(unsigned int threadID, TempResults *partialResults);

#endif /* SHAREDREGION_H */
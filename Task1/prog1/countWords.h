/**
 *  \file countWords.h 
 *  \brief Problem name: Text processing in Portuguese
 *  This module implements and stores information shared by the main and worker thread
 * 
 *
 *  \author João Morais and Miguel Ferreira
 */
#ifndef COUNTWORDS_H_
#define COUNTWORDS_H_

#include "dataStructures.h"

/**
 *  \brief Counts the number of words and words with A, E, I, O, U, Y and Ç
 * 
 *  \param in chunk of data received by the worker
 *  \param out partial results of this chunk
 */
extern void count(Chunk *in, TempResults *out);

#endif /* COUNTWORDS_H_ */
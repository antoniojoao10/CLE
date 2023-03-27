/**
 *  \file fifo.h (interface file)
 *
 *  \brief Problem name: Text processing in Portuguese
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
 *  \author João Morais and Miguel Ferreira
 */

#ifndef FIFO_H
#define FIFO_H
#include "dataStructures.h"

/**
 *  \brief Store a value in the data transfer region.
 *
 *
 *  \param prodId producer identification
 *  \param val chunk of data to save
 *  \param lt flag
 */

extern void putChunk (unsigned int prodId, Chunk val, bool lt);


/**
 *  \brief Get a value from the data transfer region.
 *
 *  \param consId worker identification
 * \param res return the chunk of data
 *
 *  \return state
 */
extern int getChunk (unsigned int consId, Chunk* res);

#endif /* FIFO_H */

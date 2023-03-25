#ifndef DATASTRUCT_H
#define DATASTRUCT_H
/**
 * \brief Struct to store a chunk of a file
 *
 */
typedef struct
{
    int numBytes;
    int fileID;
    int textChunk[6000];
} Chunk;
/**
 * \brief Struct to store the partial results from a worker thread.
 *
 */
typedef struct
{
    int fileID;
    int nWords;
    int a;
    int e;
    int i;
    int o;
    int u;
} TempResults;
#endif /* DATASTRUCT_H */
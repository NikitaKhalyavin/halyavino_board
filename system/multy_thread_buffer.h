#ifndef MULTY_THREAD_BUFFER
#define MULTY_THREAD_BUFFER

#include <stdint.h>

typedef enum {BUFFER_IS_BUSY, BUFFER_IS_FREE} BufferControlStatus;
typedef enum {BUFFER_DATA_FULL, BUFFER_DATA_EMPTY} BufferDataStatus;
typedef enum {BUFFER_CAPTURING_SUCCESSFUL, BUFFER_CAPTURING_FAIL_BUSY, BUFFER_CAPTURING_FAIL_FULL} BufferCapturingStatus;
typedef enum {BUFFER_WRITING_SUCCESFULL, BUFFER_WRITING_TOO_LONG, BUFFER_WRITING_BUSY, BUFFER_WRITING_FULL} BufferWritingResult;
typedef enum {BUFFER_READING_SUCCESFULL, BUFFER_READING_TOO_LONG, BUFFER_READING_BUSY, BUFFER_READING_EMPTY} BufferReadingResult;

typedef BufferWritingResult (*TryToWriteBuffer)(void* this, uint8_t* data, uint32_t size);
typedef BufferReadingResult (*TryToReadBuffer)(void* this, uint8_t* data, uint32_t size);

typedef struct 
{
    //data access metods
    volatile BufferControlStatus controlStatus;
    volatile BufferDataStatus dataStatus;
    TryToWriteBuffer tryToWrite;
    TryToReadBuffer tryToRead;
    
    //-------------------------------------------------------------------
    //private values
    //-------------------------------------------------------------------
    //data storage
    uint8_t *data;
    //size of data storage
    uint32_t size;
    
}DataBuffer;

void bufferInit(DataBuffer* buffer, uint8_t* memory, uint32_t size);

#endif

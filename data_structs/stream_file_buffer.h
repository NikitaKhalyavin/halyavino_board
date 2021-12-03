#ifndef STREAM_FILE_BUFFER_H_
#define STREAM_FILE_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include "common_file_defines.h"

typedef enum {STREAM_BUFFER_STATUS_IDLE, STREAM_BUFFER_STATUS_IN_PROCESS, 
            STREAM_BUFFER_STATUS_FILE_END, STREAM_BUFFER_STATUS_ERROR} StreamBufferStatus;

typedef FileSize (*StreamBufferReadData)(void* this, uint8_t* dest, FileSize size);
typedef FileSize (*StreamBufferWriteData)(void* this, const uint8_t* source, FileSize size);

typedef void (*StreamBufferReset)(void* this);
typedef void (*StreamBufferSetStatus)(void* this, StreamBufferStatus status);

typedef struct
{
    FileSize size;
    
    StreamBufferStatus status;
    
    // functions
    StreamBufferReadData read;
    StreamBufferWriteData write;
    StreamBufferWriteData writeLast;
    StreamBufferReset reset;
    StreamBufferSetStatus setStatus;
    
    // private members - do not touch
    uint8_t* dataBuffer;
    FileSize dataBufferSize;
    
    // start of writed data (internal buffer is circular)
    uint32_t startIndex;
}StreamFileBuffer;


// preparting the buffer to work
void streamFileBufferInit(StreamFileBuffer* buffer, uint8_t* data, FileSize size);

#endif

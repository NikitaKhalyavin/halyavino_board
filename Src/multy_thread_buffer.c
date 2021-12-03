#include "multy_thread_buffer.h"
#include "critical.h"
#include <stdlib.h>
#include <string.h>

BufferCapturingStatus tryToCaptureBuffer(DataBuffer* buffer)
{
    BufferCapturingStatus result;
    ENTER_CRITICAL()
    if(buffer->controlStatus != BUFFER_IS_BUSY)
    {
        buffer->controlStatus = BUFFER_IS_BUSY;
        result = BUFFER_CAPTURING_SUCCESSFUL;
    }
    else
    {
        result = BUFFER_CAPTURING_FAIL_BUSY;
    }
    EXIT_CRITICAL()
    return result;
}

void freeBuffer(DataBuffer* buffer)
{
    buffer->controlStatus = BUFFER_IS_FREE;
}

BufferWritingResult tryToWriteBuffer(void* this, uint8_t* data, uint32_t size)
{
    DataBuffer* buffer = (DataBuffer*)this;
    if(size > buffer->size)
        return BUFFER_WRITING_TOO_LONG;
    BufferCapturingStatus cap = tryToCaptureBuffer(buffer);
    if(cap != BUFFER_CAPTURING_SUCCESSFUL)
        return BUFFER_WRITING_BUSY;
    if(buffer->dataStatus == BUFFER_DATA_FULL)
    {
        freeBuffer(buffer);
        return BUFFER_WRITING_FULL;
    }
    
    memcpy(buffer->data, data, size * sizeof(uint8_t)); 

    buffer->dataStatus = BUFFER_DATA_FULL;

    
    buffer->dataStatus = BUFFER_DATA_FULL;
    freeBuffer(buffer);
    return BUFFER_WRITING_SUCCESFULL;
}


BufferReadingResult tryToReadBuffer(void* this, uint8_t* data, uint32_t size)
{
    DataBuffer* buffer = (DataBuffer*)this;
    if(size > buffer->size)
        return BUFFER_READING_TOO_LONG;
    BufferCapturingStatus cap = tryToCaptureBuffer(buffer);
    if(cap != BUFFER_CAPTURING_SUCCESSFUL)
        return BUFFER_READING_BUSY;
    if(buffer->dataStatus == BUFFER_DATA_EMPTY)
    {
        freeBuffer(buffer);
        return BUFFER_READING_EMPTY;
    }  
    
    memcpy(data, buffer->data, size * sizeof(uint8_t));    
    buffer->dataStatus = BUFFER_DATA_EMPTY;
    
    //return control
    freeBuffer(buffer);
    return BUFFER_READING_SUCCESFULL;
}

void bufferInit(DataBuffer* buffer, uint8_t* memory, uint32_t size)
{
    buffer->data = memory;
    buffer->size = size;
    buffer->tryToWrite = tryToWriteBuffer;
    buffer->tryToRead = tryToReadBuffer;
    buffer->controlStatus = BUFFER_IS_FREE;
    buffer->dataStatus = BUFFER_DATA_EMPTY;
}


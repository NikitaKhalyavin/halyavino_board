#include <stdlib.h>
#include <string.h>

#include "stream_file_buffer.h"
#include "critical.h"

static FileSize readDataFromStreamBuffer(void* this, uint8_t* dest, uint32_t size);
static FileSize writeDataToStreamBuffer(void* this, const uint8_t* source, FileSize size);
static FileSize writeDataToStreamBufferLast(void* this, const uint8_t* source, FileSize size);
static void resetStreamBuffer(void* this);
static void setStatusStreamBuffer(void* this, StreamBufferStatus status);

void streamFileBufferInit(StreamFileBuffer* buffer, uint8_t* data, FileSize size)
{
    buffer->startIndex = 0;
    buffer->dataBufferSize = size;
    buffer->dataBuffer = data;
    
    buffer->read = readDataFromStreamBuffer;
    buffer->write = writeDataToStreamBuffer;
    buffer->writeLast = writeDataToStreamBufferLast;
    buffer->reset = resetStreamBuffer;
    buffer->setStatus = setStatusStreamBuffer;
    
    buffer->size = 0;
    buffer->status = STREAM_BUFFER_STATUS_IDLE;
}


static FileSize readDataFromStreamBuffer(void* this, uint8_t* dest, uint32_t size)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    
    ENTER_CRITICAL()
    if((buffer->status != STREAM_BUFFER_STATUS_IN_PROCESS) && (buffer->status != STREAM_BUFFER_STATUS_FILE_END))
        return 0;
    
    // if can't return size bytes, return as many as possible
    if(size > buffer->size)
        size = buffer->size;
    
    
    if(buffer->startIndex + size < buffer->dataBufferSize)
    {
        //if there isn't moving throagth the start of reading section
        
        //copy data to destination
        memcpy(dest, &(buffer->dataBuffer[buffer->startIndex]), size * sizeof(uint8_t));
        
        //update internal data
        buffer->startIndex += size;
    }
    
    else 
    {
        FileSize sizeBeforeBorder = buffer->dataBufferSize - buffer->startIndex;
        FileSize sizeAfterBorder = size - sizeBeforeBorder;
        //copy data to destination
        memcpy(dest, &(buffer->dataBuffer[buffer->startIndex]), 
                    sizeBeforeBorder * sizeof(uint8_t));
        memcpy(dest + sizeBeforeBorder, buffer->dataBuffer, 
                    sizeAfterBorder * sizeof(uint8_t));
        
        //update internal data
        buffer->startIndex = sizeAfterBorder;
    }
    
    buffer->size -= size;
    EXIT_CRITICAL()
    return size;
    
}


static FileSize writeDataToStreamBuffer(void* this, const uint8_t* src, FileSize size)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    
    ENTER_CRITICAL()
    if(buffer->status != STREAM_BUFFER_STATUS_IN_PROCESS)
        return 0;
    
    // if can't write size bytes, write as many as possible
    FileSize freeSpace = buffer->dataBufferSize - buffer->size;
    if(size > freeSpace)
        size = freeSpace;
    
    uint32_t endIndex = buffer->startIndex + buffer->size;
    if(endIndex > buffer->dataBufferSize)
    {
        // border crossing in data section
        endIndex -= buffer->dataBufferSize;
    }
    
    uint32_t writeSectionEndIndex = endIndex + size;
    if(writeSectionEndIndex < buffer->dataBufferSize)
    {
        memcpy(buffer->dataBuffer + endIndex, src, size);
    }
    else
    {
        uint32_t beforeBorder = buffer->dataBufferSize - endIndex;
        uint32_t afterBorder = size - beforeBorder;
        
        memcpy(buffer->dataBuffer + endIndex, src, beforeBorder);
        memcpy(buffer->dataBuffer, src + beforeBorder, afterBorder);
    }
    
    buffer->size += size;
    EXIT_CRITICAL()
    return size;
}

static FileSize writeDataToStreamBufferLast(void* this, const uint8_t* src, FileSize size)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    FileSize sizeRet;
    ENTER_CRITICAL()
    sizeRet = writeDataToStreamBuffer(this, src, size);
    buffer->setStatus(buffer, STREAM_BUFFER_STATUS_FILE_END);
    EXIT_CRITICAL()
    return sizeRet;
}

static void resetStreamBuffer(void* this)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    ENTER_CRITICAL()
    buffer->startIndex = 0;
    buffer->size = 0;
    buffer->status = STREAM_BUFFER_STATUS_IDLE;
    EXIT_CRITICAL()
}

static void setStatusStreamBuffer(void* this, StreamBufferStatus status)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    ENTER_CRITICAL()
    buffer->status = status;
    EXIT_CRITICAL()
}


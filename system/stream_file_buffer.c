#include "stream_file_buffer.h"
#include "event_queue.h"
#include <stdlib.h>
#include <string.h>
#include <fatfs.h>
extern EventQueue eventQueue;

void bufferLowDataCallback(void* this);
StreamBufferReadingResult readDataFromBuffer(void* this, uint8_t* dest, uint32_t size);

StreamBufferInitializingResult streamFileBufferInit(StreamFileBuffer* buffer, uint32_t size, FileStorageDevice device, const char* fileName)
{
    buffer->startIndex = 0;
    buffer->dataSize = 0;
    buffer->source = device;
    buffer->isFileEnded = false;
    buffer->read = readDataFromBuffer;
        
    buffer->dataBuffer = malloc(size * sizeof(uint8_t));
    if(buffer->dataBuffer == NULL)
    {
        //can't allocate memory
        buffer->status = STREAM_BUFFER_NO_MEMORY_ERROR;
        buffer->size = 0;
        return STREAM_BUFFER_INITIALIZING_ERROR_NO_MEM;
    }
    
    FRESULT res = f_open(&(buffer->linkedObject.file), fileName,  FA_READ);
    if(res != FR_OK)
    {
        buffer->status = STREAM_BUFFER_RUNTIME_ERROR;
        return STREAM_BUFFER_INITIALIZING_ERROR_FILE_NOT_EXIST;
    }
    
    buffer->size = size;
    buffer->status = STREAM_BUFFER_OK;

    // get first data part
    bufferLowDataCallback((void*)buffer);
    
    return STREAM_BUFFER_INITIALIZING_OK;
}

void deleteBuffer(StreamFileBuffer* buffer)
{
    if(buffer->source == FILE_DEVICE_MEMORY)
    {
        FRESULT res = f_close(&(buffer->linkedObject.file));
    }
    free(buffer->dataBuffer);
}

StreamBufferReadingResult readDataFromBuffer(void* this, uint8_t* dest, uint32_t size)
{
    StreamFileBuffer* buffer = (StreamFileBuffer*)this;
    
    if(buffer->status != STREAM_BUFFER_OK)
        return STREAM_BUFFER_READING_ERROR;
    
    if(size > buffer->dataSize)
        return STREAM_BUFFER_READING_EMPTY;
    
    if(buffer->startIndex + size < buffer->size)
    {
        //if there isn't moving throagth the start of reading section
        
        //copy data to destination
        memcpy(dest, &(buffer->dataBuffer[buffer->startIndex]), size * sizeof(uint8_t));
        
        //update internal data
        buffer->startIndex += size;
    }
    
    else 
    {
        //copy data to destination
        memcpy(dest, &(buffer->dataBuffer[buffer->startIndex]), 
                (buffer->size - buffer->startIndex) * sizeof(uint8_t));
        memcpy(&dest[buffer->size - buffer->startIndex], buffer->dataBuffer, 
                (size + buffer->startIndex - buffer->size) * sizeof(uint8_t));
        
        //update internal data
        buffer->startIndex = size + buffer->startIndex - buffer->size;
    }
    
    buffer->dataSize -= size;
    
    if(buffer->dataSize < buffer->size / 2)
    {
        if(!buffer->isFileEnded)
        {
            EventMessage event;
            event.handlerReference = bufferLowDataCallback;
            event.eventParams = this;
            EnqueueResult enqueueRes = eventQueue.tryToEnque((void*)(&eventQueue), event);
            if(enqueueRes == ENQUEUE_RESULT_FULL)
            {
                //buffer will try this next time
                return STREAM_BUFFER_READING_SUCCESS;
            }
        }
    }
    
    return STREAM_BUFFER_READING_SUCCESS;
}

void bufferLowDataCallback(void* this)
{
    
    StreamFileBuffer* buffer = (StreamFileBuffer*) this;
    
    if(buffer->isFileEnded)
        //nothing to read
        return;
    
    int32_t canRead = buffer->size - buffer->dataSize;
    if(canRead <= 0)
        return;
    
    
    int32_t firstPartSize = buffer->size - buffer->startIndex - buffer->dataSize;
    if(firstPartSize < 0)
        firstPartSize = 0;
    uint8_t* firstPartPointer = &(buffer->dataBuffer[buffer->startIndex + buffer->dataSize]);
    
    int32_t secondPartSize = buffer->startIndex;
    uint8_t* secondPartPointer = buffer->dataBuffer;
    
    if(buffer->source == FILE_DEVICE_MEMORY)
    {
        FRESULT res;
        uint32_t readedBytes;
        if(firstPartSize != 0)
        {
            res = f_read(&(buffer->linkedObject.file), firstPartPointer, 
                firstPartSize, &readedBytes);
            if(res != FR_OK)
                return;
            buffer->dataSize += readedBytes;
            if(readedBytes < firstPartSize)
            {
                //end of file
                buffer->isFileEnded = true;
                return;
            }
        }
        if(secondPartSize != 0)
        {
            res = f_read(&(buffer->linkedObject.file), secondPartPointer, 
                secondPartSize, &readedBytes);
            if(res != FR_OK)
                return;
            buffer->dataSize += readedBytes;
            if(readedBytes < secondPartSize)
            {
                //end of file
                buffer->isFileEnded = true;
                return;
            }
        }
            
    }
}

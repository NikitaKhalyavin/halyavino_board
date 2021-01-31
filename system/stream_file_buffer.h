#ifndef STREAM_FILE_BUFFER_H
#define STREAM_FILE_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <fatfs.h>
#include "file_storage_device.h"

typedef enum {STREAM_BUFFER_READING_SUCCESS, STREAM_BUFFER_READING_EMPTY, STREAM_BUFFER_READING_ERROR} StreamBufferReadingResult;
typedef enum {STREAM_BUFFER_INITIALIZING_OK, STREAM_BUFFER_INITIALIZING_ERROR_NO_MEM, STREAM_BUFFER_INITIALIZING_ERROR_QUEUE_OVERFLOW, 
        STREAM_BUFFER_INITIALIZING_ERROR_FILE_NOT_EXIST} StreamBufferInitializingResult;
typedef enum {STREAM_BUFFER_OK, STREAM_BUFFER_NO_MEMORY_ERROR, STREAM_BUFFER_RUNTIME_ERROR} StreamBufferStatus;

typedef StreamBufferReadingResult (*ReadDataFromBuffer)(void* this, uint8_t* dest, uint32_t size);
typedef void (*BufferLowDataCallback)(void* data);

typedef struct
{
    uint32_t size;
    union
    {
        FIL file;
    } linkedObject;
    bool isFileEnded;
    bool isNewPacketRequested;
    
    StreamBufferStatus status;
    FileStorageDevice source;
    
    ReadDataFromBuffer read;
    
    uint8_t* dataBuffer;
    uint32_t startIndex;
    uint32_t dataSize;
}StreamFileBuffer;

StreamBufferInitializingResult streamFileBufferInit(StreamFileBuffer* buffer, uint32_t size, FileStorageDevice device, const char * fileName);
void deleteBuffer(StreamFileBuffer* buffer);

#endif

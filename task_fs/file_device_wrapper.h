#ifndef FILE_DEVICE_WRAPPER_H_
#define FILE_DEVICE_WRAPPER_H_

#include "common_file_defines.h"
#include "stream_file_buffer.h"
#include "emulated_file.h"
#include "ff.h"
#include "fs_api.h"

typedef uint32_t size_t;

typedef enum {FILE_DEVICE_WRAPPER_LINKING_SUCCESS, FILE_DEVICE_WRAPPER_LINKING_ERROR_NOFILE, 
        FILE_DEVICE_WRAPPER_LINKING_ERROR_FILE_BUSY, FILE_DEVICE_WRAPPER_LINKING_ERROR_NOT_ALLOWED} FileDeviceWrapperLinkingResult;

typedef enum {FILE_DEVICE_WRAPPER_STATUS_IDLE, FILE_DEVICE_WRAPPER_STATUS_IN_PROCESS, FILE_DEVICE_WRAPPER_STATUS_ENDED } FileDeviceWrapperStatus;

        
typedef FileDeviceWrapperLinkingResult (*FileDeviceWrapperLinkFile)(void* this, const char* fileName, 
            FileStorageDevice source, FileTransferDirection dir);
typedef void (*FileDeviceWrapperSetReceivingBuffer)(void* this, StreamFileBuffer* buffer);
typedef void (*FileDeviceWrapperRunTransferIfPossible)(void* this);
typedef void (*FileDeviceWrapperReset)(void* this);
typedef void (*FileDeviceWrapperCloseFile)(void* this);

typedef size_t (*FileDeviceWrapperReadData)(void* this, uint8_t* dest, size_t size);
typedef size_t (*FileDeviceWrapperWriteData)(void* this, const uint8_t* source, size_t size);

typedef struct 
{
    FileDeviceWrapperStatus status;
    FileTransferDirection transferDirection;
    
    FileDeviceWrapperLinkFile linkFile;
    FileDeviceWrapperSetReceivingBuffer setBuffer;
    FileDeviceWrapperReset reset;
    
    FileDeviceWrapperRunTransferIfPossible runTransfer;
    
    // private functions
    FileDeviceWrapperReadData read;
    FileDeviceWrapperWriteData write;
    FileDeviceWrapperCloseFile closeFile;
    
    // private members
    FileStorageDevice fileDevice;
    union
    {
        FIL fatFS_File;
        EmulatedFileDescriptor emulatedFile;
    }file;
    bool isFileOpened;
    StreamFileBuffer* receivingBuffer;
} FileDeviceWrapper;

void fileDeviceWrapperInit(FileDeviceWrapper* wrapper);

// call this one before the others
void initFatFS(void);


    
// blocks thread for the whole time, do not use during execution
void overviewFileSystem(StreamFileBuffer* receivingBuffer);

FS_ApiResult deleteFileFromFS(const char* fileName);

#endif // FILE_DEVICE_WRAPPER_H_

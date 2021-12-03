#ifndef FILE_TRANSFER_CONTROL_H
#define FILE_TRANSFER_CONTROL_H
#include <stdint.h>
#include <fatfs.h>
#include "stream_file_buffer.h"
#include "file_storage_device.h"

typedef enum {FILE_TRANSFER_EMPTY, FILE_TRANSFER_INITIALIZING, FILE_TRANSFER_IN_PROGRESS, 
                            FILE_TRANSFER_FINISHED, FILE_TRANSFER_ERROR} FileTransferStatus;

                            
                           
typedef void(*StartFileTransfer)(void* this, char* fileName, FileStorageDevice source, FileStorageDevice destination);
typedef void(*StartTransferToBuffer)(void* this, char* fileName, FileStorageDevice source, StreamFileBuffer* destination);

typedef struct
{
    FileStorageDevice sourceDevice;
    FileStorageDevice destinationDevice;
    
    StartFileTransfer start;
    
    
    //private metods and members
    FileTransferStatus currentStatus;
    char* fileNameString;
    
    uint32_t fileSize;
    uint32_t packetNumber;
    uint32_t currentPacket;
    
    union
    {
      FIL file;  
    } destinationLinkedObject;
    
    union
    {
      FIL file;  
    } sourceLinkedObject;
    
} FileTransferManager;

void fileTransferManagerInit(FileTransferManager* manager);

#endif

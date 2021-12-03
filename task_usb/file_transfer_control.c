#include "file_transfer_control.h"
#include "file_request_handler.h"
#include <stdlib.h>
#include <string.h>

void packetHandler(void* this, PacketRequestResult result, uint8_t* buffer);
void startFileTransfer(void* this, char* fileName, FileStorageDevice source, FileStorageDevice destination);



void fileTransferManagerInit(FileTransferManager* manager)
{
    manager->currentStatus = FILE_TRANSFER_EMPTY;
    manager->fileNameString = (char*)malloc(sizeof(char) * 60);
    manager->start = startFileTransfer;
}

void packetHandler(void* this, PacketRequestResult result, uint8_t* buffer)
{
    FileTransferManager* manager = (FileTransferManager*)this;
    uint32_t packetNumber = *(uint32_t*)(&buffer[0]);
        
    if(packetNumber != manager->currentPacket)
        //we asked for the other one
        return;
    
    if(manager->currentStatus == FILE_TRANSFER_INITIALIZING)
    {
        //this packet is first
        if(manager->sourceDevice == FILE_DEVICE_USB)
        {
            manager->packetNumber = *(uint32_t*)(&buffer[4]);
            manager->fileSize = *(uint32_t*)(&buffer[8]);
            uint32_t bytesForWrite = 50;
            if(manager->fileSize < bytesForWrite)
                bytesForWrite = manager->fileSize;
            if(manager->destinationDevice == FILE_DEVICE_MEMORY)
            {
                uint32_t writedBytes;
                FRESULT res = f_write(&(manager->destinationLinkedObject.file), &buffer[12], bytesForWrite, &writedBytes);
                if(res != FR_OK)
                    return;
                if(writedBytes < bytesForWrite)
                    return;
            }
    
        }
        manager->currentStatus = FILE_TRANSFER_IN_PROGRESS;
    }
    
    else
    {
        if(manager->sourceDevice == FILE_DEVICE_USB)
        {
            uint32_t bytesAlreadyWrited = 50 + 58 * (manager->currentPacket - 1);
            uint32_t bytesForWrite = 58;
            if(manager->fileSize - bytesAlreadyWrited < bytesForWrite)
                bytesForWrite = manager->fileSize - bytesAlreadyWrited;
            if(manager->destinationDevice == FILE_DEVICE_MEMORY)
            {
                uint32_t writedBytes;
                FRESULT res = f_write(&(manager->destinationLinkedObject.file), &buffer[4], bytesForWrite, &writedBytes);
                if(res != FR_OK)
                    return;
                if(writedBytes < bytesForWrite)
                    return;
            }
    
        }
    }
    
    manager->currentPacket++;
    if(manager->currentPacket == manager->packetNumber)
    {
        //this packet was the last one
        //transmission completed, close the file and destroy itself
        if(manager->destinationDevice == FILE_DEVICE_MEMORY)
        {
            FRESULT res = f_close(&(manager->destinationLinkedObject.file));
            if(res != FR_OK)
            {
                //TO-DO: error handling
            }
        }
        
        free(manager->fileNameString);
        free(manager);
        return;
    }
    if(manager->sourceDevice == FILE_DEVICE_USB)
    {
        RequestMessage request;
        request.callback = packetHandler;
        request.fileName = manager->fileNameString;
        request.packetNumber = manager->currentPacket;
        request.owner = this;
        sendFileRequest(request);
    }
}


void startFileTransfer(void* this, char* fileName, FileStorageDevice source, FileStorageDevice destination)
{    
    FileTransferManager* manager = (FileTransferManager*)this;
    if(manager->currentStatus != FILE_TRANSFER_EMPTY)
        // TO-DO: error handling
        return;
    
    memcpy(manager->fileNameString, fileName, strlen(fileName) + 1);  //+1 for \0
    manager->currentStatus = FILE_TRANSFER_IN_PROGRESS;
    manager->destinationDevice = destination;
    manager->sourceDevice = source;
    
    
    if(manager->destinationDevice == FILE_DEVICE_MEMORY)
    {
        FRESULT res;
        for(int index = 0; index < strlen(manager->fileNameString); index++)
        {
            if(manager->fileNameString[index] == '\\' )
            {
                char dirName[index + 1];
                memcpy(dirName, manager->fileNameString, index);
                dirName[index] = '\0';
                
                FILINFO fil;
                res = f_stat(dirName, &fil);
                if((res == FR_NO_PATH) || (res == FR_NO_FILE))
                {     
                    res = f_mkdir(dirName);
                    if(res != FR_OK)
                        return;
                }
            }
        }
        res = f_open(&(manager->destinationLinkedObject.file), manager->fileNameString, FA_WRITE | FA_CREATE_ALWAYS);
        if(res != FR_OK)
            return;
    }
    
    //0, because we have to receive at least first packet to know how large the file is
    manager->currentPacket = 0;
    if(manager->sourceDevice == FILE_DEVICE_USB)
    {
        RequestMessage request;
        request.callback = packetHandler;
        request.fileName = manager->fileNameString;
        request.packetNumber = manager->currentPacket;
        request.owner = this;
        sendFileRequest(request);
        manager->currentStatus = FILE_TRANSFER_INITIALIZING;
    }
    
}


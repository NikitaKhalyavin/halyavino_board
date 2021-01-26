
#include "usbd_custom_hid_if.h"
#include "file_request_handler.h"
#include "usb_proto_defines.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fatfs.h>

extern USBD_HandleTypeDef hUsbDeviceFS;


uint8_t tempBuffer[63];
uint8_t sendBuffer[64];
uint8_t responseBuffer[64];

FileRequestHandlerStatus currentStatus = FILE_REQUEST_HANDLER_FREE;

RequestMessage currentRequest;
const int32_t REQUEST_QUEUE_SIZE = 10;
static RequestMessage requestQueue[REQUEST_QUEUE_SIZE];
int32_t queueBeginPointer = 0;
int32_t queueEndPointer = 0;

int32_t getQueueLength()
{
    int32_t currentLength = queueEndPointer - queueBeginPointer;
    if(currentLength < 0)
        currentLength += REQUEST_QUEUE_SIZE;
    return currentLength;
}

void enqueueReq(RequestMessage message)
{
    int32_t currentLength = getQueueLength();
    if(currentLength > REQUEST_QUEUE_SIZE)
        return;
    
    requestQueue[queueEndPointer] = message;
    queueEndPointer++;
    if(queueEndPointer > REQUEST_QUEUE_SIZE)
    queueEndPointer -= REQUEST_QUEUE_SIZE;
}

RequestMessage dequeueReq()
{
    int32_t currentLength = getQueueLength();
    if(currentLength == 0)
    {
        //queue is empty
        RequestMessage dummy;
        dummy.callback = NULL;
        dummy.owner = NULL;
        dummy.packetNumber = 0;
        dummy.fileName = NULL;
        return dummy;
    }
    RequestMessage result = requestQueue[queueBeginPointer];
    queueBeginPointer++;
    if(queueBeginPointer > REQUEST_QUEUE_SIZE)
        queueBeginPointer -= REQUEST_QUEUE_SIZE;
    return result;
}

void handleFirstFromQueue()
{
    int32_t currentLength = getQueueLength();
    if(currentLength == 0)
        //queue is empty
        return;
    
    currentStatus = FILE_REQUEST_HANDLER_BUSY;
    currentRequest = dequeueReq();
    
    
    uint8_t header = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_FILE |
                USB_PACKET_TYPE_REQUEST | USB_PACKET_STATUS_OK;
    sendBuffer[0] = 2;
    sendBuffer[1] = header;
    memcpy(&sendBuffer[2], &(currentRequest.packetNumber), sizeof(uint32_t));
    sendBuffer[6] = strlen(currentRequest.fileName);
    memcpy(&sendBuffer[7], currentRequest.fileName, strlen(currentRequest.fileName) + 1);
    
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sendBuffer, FULL_REPORT_LENGTH);
}


void sendFileRequest(RequestMessage request)
{
    enqueueReq(request);
    
    if(currentStatus == FILE_REQUEST_HANDLER_BUSY)
        return;
    
    handleFirstFromQueue();
    
}

bool writeFileSystemInStringRecourse(char* string, uint32_t* cursor, char* currentDirName)
{
    DIR currentDir;
    const char separator = '\t';
    const char directoryEntrySymbol = '\n';
    const char directoryExitSymbol = '\0';
    FRESULT res;
    res = f_opendir(&currentDir, currentDirName);
    if(res != FR_OK)
        return true;
    
    res = f_rewinddir(&currentDir);
    if(res != FR_OK)
        return true;
    
    FILINFO obj;
    res = f_readdir(&currentDir, &obj);
    if(res != FR_OK)
        return true;
    while(obj.fname[0] != '\0')
    {
        //while the name is not empty
        uint32_t objectNameLength = strlen(obj.fname);
        memcpy(&string[*cursor], obj.fname, objectNameLength);
        (*cursor) += objectNameLength;
        if(obj.fattrib & AM_DIR)
        {
            string[(*cursor)++] = directoryEntrySymbol;
            bool tempRes = writeFileSystemInStringRecourse(string, cursor, obj.fname);
            if(tempRes)
                return true;
        }
        else
        {
            string[(*cursor)++] = separator;
        }
        
        res = f_readdir(&currentDir, &obj);
        if(res != FR_OK)
            return true;
    }
    string[(*cursor)++] = directoryExitSymbol;
    return false;
}


void handleNewRequest(uint8_t* packet)
{    
    uint8_t header = packet[0];
    if ((header & USB_PACKET_STATUS_POSITION) == USB_PACKET_STATUS_WAIT)
    {
        //error: request cannot wait
        return;
    }
    if ((header & USB_PACKET_STATUS_POSITION) == USB_PACKET_STATUS_OK)
    {
        int packetNumber = *((int*)(&packet[1]));
        uint8_t nameLen = packet[5];
        if(nameLen == 0)
        {
            //fs scanning
            static char resultString[512];
            static uint32_t cursor;
            
            uint8_t responseHeader = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_FILE |
                USB_PACKET_TYPE_RESPONSE | USB_PACKET_STATUS_OK;
            
            if(packetNumber == 0)
            {
                //first packet - get filestruct
                cursor = 0;
                bool fsScanRes = writeFileSystemInStringRecourse(resultString, &cursor, "/");
                if(fsScanRes)
                {
                    //send error packet
                    responseHeader &= ~USB_PACKET_STATUS_POSITION;
                    responseHeader |= USB_PACKET_STATUS_ERROR;
                    responseHeader &= ~USB_PACKET_TYPE_POSITION;
                    responseHeader |= USB_PACKET_TYPE_REQUEST;
                    responseBuffer[1] = responseHeader;
                    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
                }
                cursor--; //to skip the last '\0' symbol
            }
            
            //cursor value equals string length
            int numberOfPacketsForSending;
            if(cursor <= 50)
                numberOfPacketsForSending = 1;
            else
            {
                int numberOfPacketsAfterFirst;
                
                if( (cursor - 50) % 58 == 0)
                    numberOfPacketsAfterFirst = (cursor - 50) / 58;
                else
                    numberOfPacketsAfterFirst = (cursor - 50) / 58 + 1;
                numberOfPacketsForSending = numberOfPacketsAfterFirst + 1;
            }


            responseBuffer[0] = 2;
            responseBuffer[1] = responseHeader;
            memcpy(&responseBuffer[2], &packetNumber, sizeof(uint32_t));

            
            if(packetNumber == 0)
            {
                
                memcpy(&responseBuffer[6], &numberOfPacketsForSending, sizeof(uint32_t));
                memcpy(&responseBuffer[10], &cursor, sizeof(uint32_t));
                int bytesToRead = 50;
                if(cursor < bytesToRead)
                    bytesToRead = cursor;
                memcpy(&responseBuffer[14], resultString, bytesToRead);                                
            }
            else
            {
                unsigned int readedBytesNumber = 58 * (packetNumber - 1) + 50;
                int bytesToRead = 58;
                if(cursor - readedBytesNumber < bytesToRead)
                    bytesToRead = cursor - readedBytesNumber;
                memcpy(&responseBuffer[14], &resultString[readedBytesNumber], bytesToRead);
            }
            
            //send requested packet
            USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            return;
        }
        if(nameLen > 60)
            //error: too long name
            return;
        
        char name[61];
        memcpy(name, &packet[6], nameLen);
        name[nameLen] = '\0';
        
        FIL file;
        FRESULT res = f_open(&file, name, FA_READ);
        
        if(res != FR_OK)
        {
            return;
        }
        
        int fileSize = file.fsize;
        int numberOfPackets;
        if(fileSize <= 50)
            numberOfPackets = 1;
        else
        {
            int numberOfPacketsAfterFirst;
            
            if( (fileSize - 50) % 58 == 0)
                numberOfPacketsAfterFirst = (fileSize - 50) / 58;
            else
                numberOfPacketsAfterFirst = (fileSize - 50) / 58 + 1;
            
            numberOfPackets = numberOfPacketsAfterFirst + 1;
        }
        
        uint8_t responseHeader = USB_PACKET_DIRECTION_FROM_DEVICE | USB_PACKET_COMMAND_TYPE_FILE |
                USB_PACKET_TYPE_RESPONSE | USB_PACKET_STATUS_OK;
        responseBuffer[0] = 2;
        responseBuffer[1] = responseHeader;
        memcpy(&responseBuffer[2], &packetNumber, sizeof(uint32_t));
            
        if(packetNumber == 0)
        {
            memcpy(&responseBuffer[6], &numberOfPackets, sizeof(uint32_t));
            memcpy(&responseBuffer[10], &fileSize, sizeof(uint32_t));
            int bytesToRead = 50;
            if(fileSize < bytesToRead)
                bytesToRead = fileSize;
            uint32_t readedBytes;
         
            res = f_read(&file, &responseBuffer[14], bytesToRead, &readedBytes);
            if( (res == FR_OK) && (readedBytes == bytesToRead) )
            {
                //send requested packet
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            }
            else
            {
                //send error packet
                responseHeader &= ~USB_PACKET_STATUS_POSITION;
                responseHeader |= USB_PACKET_STATUS_ERROR;
                responseHeader &= ~USB_PACKET_TYPE_POSITION;
                responseHeader |= USB_PACKET_TYPE_REQUEST;
                responseBuffer[1] = responseHeader;
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            }
        }
        else
        {
            unsigned int readedBytesNumber = 58 * (packetNumber - 1) + 50;
            int bytesToRead = 58;
            if(fileSize - readedBytesNumber < bytesToRead)
                bytesToRead = fileSize - readedBytesNumber;
            uint32_t readedBytes;
            res = f_lseek(&file, readedBytesNumber);
            if( res != FR_OK)
            {
                //send error packet
                responseHeader &= ~USB_PACKET_STATUS_POSITION;
                responseHeader |= USB_PACKET_STATUS_ERROR;
                responseHeader &= ~USB_PACKET_TYPE_POSITION;
                responseHeader |= USB_PACKET_TYPE_REQUEST;
                responseBuffer[1] = responseHeader;
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            }
            res = f_read(&file, &responseBuffer[6], bytesToRead, &readedBytes);
            if( (res == FR_OK) && (readedBytes == bytesToRead) )
            {
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            }
            else
            {
                //send error packet
                responseHeader &= ~USB_PACKET_STATUS_POSITION;
                responseHeader |= USB_PACKET_STATUS_ERROR;
                responseHeader &= ~USB_PACKET_TYPE_POSITION;
                responseHeader |= USB_PACKET_TYPE_REQUEST;
                responseBuffer[1] = responseHeader;
                USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, responseBuffer, FULL_REPORT_LENGTH);
            }        
        }
        res = f_close(&file);
        if(res != FR_OK)
            //all is very bad
            return;
        //all is bad
        return;
    }
}


void handleNewResponse(uint8_t* packet)
{
    if(currentStatus == FILE_REQUEST_HANDLER_FREE)
    {
        //error: there are no active requests
        return;
    }
    if( (currentRequest.callback == NULL) || (currentRequest.owner == NULL) )
    {
        //nothing to call
        return;
    }
    
    currentRequest.callback(currentRequest.owner, PACKET_REQUEST_SUCCESS, &packet[1]);
    currentStatus = FILE_REQUEST_HANDLER_FREE;
    handleFirstFromQueue();
}

void repeatLastSended()
{
    if(currentStatus == FILE_REQUEST_HANDLER_BUSY)
        //other way, we did not received anything
        USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, sendBuffer, FULL_REPORT_LENGTH);
}


void terminateTransmission()
{
    if(currentStatus == FILE_REQUEST_HANDLER_BUSY)
    {
        currentRequest.callback(currentRequest.owner, PACKET_REQUEST_ERROR, NULL);
        currentStatus = FILE_REQUEST_HANDLER_FREE;
    }
    handleFirstFromQueue();
}


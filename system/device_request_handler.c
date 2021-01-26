#include "device_request_handler.h"
#include "file_request_handler.h"
#include "usb_proto_defines.h"
#include "file_transfer_control.h"
#include <global_device_manager.h>
#include <led_hardware_driver.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fatfs.h>


static char fileName[MAX_FILE_NAME_LENGTH + 1] = {0};

void handleDeviceRequest(uint8_t* request)
{
    
    uint8_t fileNameLength = request[3];
    if(fileNameLength > MAX_FILE_NAME_LENGTH)
        //error, it's impossible. Stop process
        return;
    memcpy(fileName, &(request[4]), fileNameLength);
    fileName[fileNameLength] = 0;
    
    uint8_t deviceDescriptor = request[1];
    uint8_t fileDescriptor = request[2];
    
    if((deviceDescriptor & DEVICE_TYPE_POSITION) == DEVICE_TYPE_MEMORY)
    {
        if(GET_DEVICE_ADDRESS(deviceDescriptor) != 0)
            //error: it must be impossible
            return;
        if((fileDescriptor & FILE_ACTION_POSITION) == FILE_ACTION_LOAD)
        {
            if((fileDescriptor & FILE_SOURCE_POSITION) == FILE_SOURCE_USB)
            {   
                FileTransferManager* newFileTransferManager = (FileTransferManager*)malloc(sizeof(FileTransferManager));
                fileTransferManagerInit(newFileTransferManager);
                
                newFileTransferManager->start(newFileTransferManager, fileName, FILE_DEVICE_USB, FILE_DEVICE_MEMORY);
                return;
            }
            //TO-DO: all other
        }
        else
        {
            if((fileDescriptor & FILE_SOURCE_POSITION) == 0)
            {   
                // there is no source during deletion
                FRESULT res;
                FILINFO fileDescriptor;
                res = f_stat(fileName, &fileDescriptor);
                if(res != FR_OK)
                    //the file does not exist
                    return;
                res = f_unlink(fileName);
                if(res != FR_OK)
                    //something happened
                    return;
                
                bool canParentDirectoryBeEmpty = true;
                while(canParentDirectoryBeEmpty)
                {
                    int nameLen = strlen(fileName);
                    canParentDirectoryBeEmpty = false;
                    for(int index = nameLen; index > 0; index--)
                    {
                        if(fileName[index] == '\\')
                        {
                            // the parent directory is found
                            fileName[index] = '\0';
                            DIR currentDir;
                            res = f_opendir(&currentDir, fileName);
                            if(res != FR_OK)
                                return;
                            
                            res = f_rewinddir(&currentDir);
                            if(res != FR_OK)
                                return;
                            
                            FILINFO obj;
                            res = f_readdir(&currentDir, &obj);
                            if(res != FR_OK)
                                return;
                            if(obj.fname[0] != '\0')
                            {
                                //this directory has at least one child. stop
                                return;
                            }
                            res = f_unlink(fileName);    
                            if(res != FR_OK)
                                return;
                            canParentDirectoryBeEmpty = true;
                            break;
                        }
                    }
                }
                
                FileTransferManager* newFileTransferManager = (FileTransferManager*)malloc(sizeof(FileTransferManager));
                fileTransferManagerInit(newFileTransferManager);
                
                newFileTransferManager->start(newFileTransferManager, fileName, FILE_DEVICE_USB, FILE_DEVICE_MEMORY);
                return;
            }
        }
    }
    
    
    if((deviceDescriptor & DEVICE_TYPE_POSITION) == DEVICE_TYPE_LED)
    {
        uint8_t deviceAddress = GET_DEVICE_ADDRESS(deviceDescriptor);
        LedChannel channel;
        switch(deviceAddress)
        {
            case 0:
                channel = LED_CHANNEL_1;
                break;
            case 1:
                channel = LED_CHANNEL_2;
                break;
            case 2:
                channel = LED_CHANNEL_3;
                break;
            case 3:
                channel = LED_CHANNEL_4;
                break;
            case 4:
                channel = LED_CHANNEL_5;
                break;
            case 5:
                channel = LED_CHANNEL_6;
                break;
            default:
            //errror: unknown device
            return;
        }
        
        if((fileDescriptor & FILE_ACTION_POSITION) == FILE_ACTION_LOAD)
        {
            
            if((fileDescriptor & FILE_SOURCE_POSITION) == FILE_SOURCE_MEMORY)
            {                   
                setLedFile(channel, fileName);
                return;
            }
            //TO-DO: all other
        }
        else
        {
            //it must be impossible
            return;
        }
    }
    
    //TO_DO: all other
        
}


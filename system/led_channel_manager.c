#include "led_channel_manager.h"
#include <stdlib.h>

static void linkFile(void * this, const char* fileName)
{
    LedChannelManager * manager = (LedChannelManager*) this;
    
    if(manager->status != DEVICE_STATUS_FREE)
    {
        //error: file is already linked or error was occured
        return;
    }
    
    void* filePointer = malloc(sizeof(LedFileDescriptor));
    if(filePointer == NULL)
    {
        //end of memory
        manager->status = DEVICE_STATUS_ERROR;
        return;
    }
    manager->linkedFile = (LedFileDescriptor*)filePointer;
    
    ledFileDescriptorInit(manager->linkedFile);
    
    //TO_DO: error message processing
    manager->linkedFile->readFile(manager->linkedFile, fileName);
    if(manager->linkedFile->status == FILE_RECORDING)
    {
        //success
        manager->status = DEVICE_STATUS_WAIT;
        return;
    }
    else
    {
        manager->status = DEVICE_STATUS_ERROR; 
    }
    
}

static void ledChannelEndRecording(void* this)
{
    LedChannelManager * manager = (LedChannelManager*) this;
    manager->linkedFile->close((void*)(manager->linkedFile));
    free(manager->linkedFile);    
    ledHardwareDeinit(manager->channel);
    manager->status = DEVICE_STATUS_FREE;
}

static void ledChannelStartRecording(void* this)
{
    LedChannelManager * manager = (LedChannelManager*) this;
    if(manager->linkedFile == NULL)
    {
        // no file linked
        manager->status = DEVICE_STATUS_ERROR;
        return;
    }
    if((manager->linkedFile->status == FILE_EMPTY) || 
        (manager->linkedFile->status == FILE_ERROR))
    {
        //unable to record an empty file
        manager->status = DEVICE_STATUS_ERROR;
        return;
    }

    ledHardwareInit(manager->channel);
    manager->status = DEVICE_STATUS_RECORDING;
}


static void setHardwarePWM_Value(void * this, float timeFromBeginning)
{
    LedChannelManager * manager = (LedChannelManager*) this;
    if(manager->linkedFile->status == FILE_EMPTY)
    {
        manager->endRecording(this);
    }
    uint32_t channelValue = manager->linkedFile->getChannelValue((void*)(manager->linkedFile), timeFromBeginning);
    setChannelPWM_Value(manager->channel, channelValue);    
}

void ledChannelManagerInit(LedChannelManager* this)
{
    this->linkFile = linkFile;
    this->startRecording = ledChannelStartRecording;
    this->endRecording = ledChannelEndRecording;
    this->setValue = setHardwarePWM_Value;
    this->linkedFile = NULL;
    this->status = DEVICE_STATUS_FREE;
}

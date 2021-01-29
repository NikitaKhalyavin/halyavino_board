#include "sound_manager.h"

#include <dac.h>
#include <stdlib.h>

static void linkFile(void * this, const char* fileName)
{
    WavChannelManager * manager = (WavChannelManager*) this;
    
    if(manager->status != DEVICE_STATUS_FREE)
    {
        //error: file is already linked or error was occured
        return;
    }
    
    void* filePointer = malloc(sizeof(WavFileDescriptor));
    if(filePointer == NULL)
    {
        //end of memory
        manager->status = DEVICE_STATUS_ERROR;
        return;
    }
    manager->linkedFile = (WavFileDescriptor*)filePointer;
    
    wavFileDescriptorInit(manager->linkedFile);
    
    //TO_DO: error message processing
    manager->linkedFile->readFile(manager->linkedFile, fileName);
    if(manager->linkedFile->status == FILE_RECORDING)
    {
        //success
        manager->status = DEVICE_STATUS_WAIT;
        setTimerSampleRate(manager->linkedFile->sampleRate);
        return;
    }
    else
    {
        manager->status = DEVICE_STATUS_ERROR; 
    }
    
}

static void wavChannelEndRecording(void* this)
{
    disableSpeaker();
    WavChannelManager * manager = (WavChannelManager*) this;
    if(manager->linkedFile != NULL)
    {
        manager->linkedFile->close((void*)(manager->linkedFile));
        free(manager->linkedFile);
        manager->linkedFile = NULL;
    }        
    manager->status = DEVICE_STATUS_FREE;
}

static void wavChannelStartRecording(void* this)
{
    WavChannelManager * manager = (WavChannelManager*) this;
    
    if(manager->status != DEVICE_STATUS_WAIT)
        // device is already running or not prepared
        return;
    
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

    enableSpeaker();
    manager->status = DEVICE_STATUS_RECORDING;
}


static void setHardwareSoundValue(void * this)
{
    WavChannelManager * manager = (WavChannelManager*) this;
    if((manager->linkedFile->status == FILE_EMPTY) || (manager->linkedFile->status == FILE_ENDED))
    {
        manager->endRecording(this);
        return;
    }
    uint32_t channelValue = manager->linkedFile->getChannelValue((void*)(manager->linkedFile), 0);
    setDacValue(channelValue);    
}

void wavChannelManagerInit(WavChannelManager* this)
{
    this->linkFile = linkFile;
    this->startRecording = wavChannelStartRecording;
    this->endRecording = wavChannelEndRecording;
    this->setValue = setHardwareSoundValue;
    this->linkedFile = NULL;
    this->status = DEVICE_STATUS_FREE;
    disableSpeaker();
}

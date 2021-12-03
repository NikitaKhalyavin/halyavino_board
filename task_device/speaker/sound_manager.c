#include "sound_manager.h"
#include "dac.h"
#include "fs_api.h"
#include "critical.h"

#include <stdlib.h>

static void speakerReset(SpeakerManager* manager)
{
		manager->samplesReaded = 0;
		if(manager->isFileLinked)
		{
				unlinkFileFromBuffer(&manager->buffer);
		}
		manager->buffer.reset(&manager->buffer);
		manager->base.status = DEVICE_STATUS_FREE;
		disableSpeaker();
}

static void speakerLinkFile(void * base, const char* fileName)
{
    DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(!baseCasted->isEnabled)
				return;
		
		if(baseCasted->type != DEVICE_TYPE_SPEAKER)
				// error
				return;
    SpeakerManager * manager = (SpeakerManager*) baseCasted->typeSpecifiedDescriptor;
    
    if(manager->base.status != DEVICE_STATUS_FREE)
    {
        //error: file is already linked or error was occured
        return;
    }
    
    FS_ApiResult res = linkFileToBuffer(&manager->buffer, fileName, 
							FILE_TRANSFER_DIRECTION_GET, FILE_DEVICE_STORAGE);
		if(res != FS_API_RESULT_SUCCESS)
		{
				speakerReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}
		manager->isFileLinked = true;
		
		FileParsingResult readRes = getWavFileHeader(&manager->buffer, &manager->header);
		if(readRes != FILE_PARSING_RESULT_SUCCESS)
		{
				// file is invalid
				speakerReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}
		manager->base.status = DEVICE_STATUS_WAIT;    
}

static void speakerEndRecording(void* base)
{
    DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_SPEAKER)
				// error
				return;
    SpeakerManager * manager = (SpeakerManager*) (baseCasted->typeSpecifiedDescriptor);
    
		speakerReset(manager);
}

static void speakerStartRecording(void* base, int32_t time)
{
    DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(!baseCasted->isEnabled)
				return;
		
		if(baseCasted->type != DEVICE_TYPE_SPEAKER)
				// error
				return;
    SpeakerManager * manager = (SpeakerManager*) (baseCasted->typeSpecifiedDescriptor);
    
		if(manager->base.status != DEVICE_STATUS_WAIT)
        // device is already running or not prepared
        return;
    
    setTimerSampleRate(manager->header.sampleRate);
		setDacValue(0);
    enableSpeaker();
		manager->statusISR = SPEAKER_STATUS_ISR_PROCESSING;
    manager->base.status = DEVICE_STATUS_RECORDING;
}

static void speakerHandle(void* base, int32_t time)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_SPEAKER)
				// error
				return;
    SpeakerManager * manager = (SpeakerManager*) (baseCasted->typeSpecifiedDescriptor);
    
		if(manager->statusISR == SPEAKER_STATUS_ISR_PROCESSING)
				// processing is handling into ISR
				return;
		
		// recording end must occur fom here
		speakerReset(manager);
		
		if(manager->statusISR == SPEAKER_STATUS_ISR_ERROR)
				manager->base.status = DEVICE_STATUS_ERROR;
}


void speakerHandleFromISR(SpeakerManager* manager)
{
		if(!manager->base.isEnabled)
		{
				disableSpeaker();
				ENTER_CRITICAL()
				manager->statusISR = SPEAKER_STATUS_ISR_IDLE;
				EXIT_CRITICAL()
		}
    if(manager->base.status != DEVICE_STATUS_RECORDING)
    {
        return;
    }
		if(manager->statusISR != SPEAKER_STATUS_ISR_PROCESSING)
		{
				// must never occur
				return;
		}
		if(manager->samplesReaded >= manager->header.numberOfSamples)
		{
				disableSpeaker();
				ENTER_CRITICAL()
				manager->statusISR = SPEAKER_STATUS_ISR_IDLE;
				EXIT_CRITICAL()
		}
		
		uint8_t value;
		FileParsingResult res = getWavFileData(&manager->buffer, &value, manager->header);
		if(res == FILE_PARSING_RESULT_SUCCESS)
		{
				setDacValue(value);
				manager->samplesReaded++;
		}
		else
		{
				disableSpeaker();
				ENTER_CRITICAL()
				manager->statusISR = SPEAKER_STATUS_ISR_ERROR;
				EXIT_CRITICAL()
		}
}

void speakerManagerInit(SpeakerManager* this)
{
		this->base.status 									= DEVICE_STATUS_FREE;
		this->base.typeSpecifiedDescriptor	= this;
		this->base.type 										= DEVICE_TYPE_SPEAKER;
		this->base.next 										= NULL;
	
		this->base.link 				= speakerLinkFile;
		this->base.handle 			= speakerHandle;
		this->base.start 				= speakerStartRecording;
		this->base.stop 				= speakerEndRecording;
		this->base.isEnabled 		= false;
		this->base.hardwareName = NULL;
	
		this->isFileLinked = false;
		this->samplesReaded = 0;
		this->statusISR = SPEAKER_STATUS_ISR_IDLE;
		streamFileBufferInit(&this->buffer, this->bufferData, SPEAKER_BUFFER_SIZE);
    disableSpeaker();
}

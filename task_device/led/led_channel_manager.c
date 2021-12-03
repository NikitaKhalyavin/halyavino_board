#include "led_channel_manager.h"
#include "led_file_parser.h"
#include "fs_api.h"

#include <stddef.h>
#include <math.h>

static void ledChannelReset(LedChannelManager* manager)
{
		manager->current.timeInSeconds 	= NAN;
		manager->current.value 					= NAN;
		manager->next.timeInSeconds 		= NAN;
		manager->next.value 						= NAN;
		manager->beginningTime 					= 0;
		if(manager->isFileLinked)
		{
				unlinkFileFromBuffer(&manager->buffer);
		}
		manager->buffer.reset(&manager->buffer);
		manager->base.status = DEVICE_STATUS_FREE;
		setChannelPWM_Value(manager->channel, 0);
		//ledHardwareDeinit(manager->channel);
}

static void ledChannelLinkFile(void * base, const char* fileName)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;	
		if(!baseCasted->isEnabled)
				return;
		
		if(baseCasted->type != DEVICE_TYPE_LED)
				// error
				return;
    LedChannelManager * manager = (LedChannelManager*) (baseCasted->typeSpecifiedDescriptor);
    
    if(manager->base.status != DEVICE_STATUS_FREE)
    {
        //error: file is already linked or error was occured
        return;
    }
    
    FS_ApiResult res = linkFileToBuffer(&manager->buffer, fileName, 
							FILE_TRANSFER_DIRECTION_GET, FILE_DEVICE_STORAGE);
		if(res != FS_API_RESULT_SUCCESS)
		{
				ledChannelReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}
		manager->isFileLinked = true;
		
		LedFileHeader header;
		FileParsingResult readRes = getLedFileHeader(&(manager->buffer), &header);
		bool errorSignature = (header.signature[0] != 'l') || (header.signature[1] != 'e') || (header.signature[2] != 'd');
		if((readRes != FILE_PARSING_RESULT_SUCCESS) || errorSignature)
		{
				// file is invalid
				ledChannelReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}
		manager->pointNumber = header.pointNumber;
		if(manager->pointNumber < 2)
		{
				// too short file, ended already
				ledChannelReset(manager);
				manager->base.status = DEVICE_STATUS_FREE;
				return;
		}
		readRes = getLedFilePoint(&(manager->buffer), &(manager->next));
		if((readRes != FILE_PARSING_RESULT_SUCCESS) || errorSignature)
		{
				// file is invalid
				ledChannelReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}
		manager->base.status = DEVICE_STATUS_WAIT;
}

static void ledChannelEndRecording(void* base)
{
    DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_LED)
				// error
				return;
    LedChannelManager * manager = (LedChannelManager*) (baseCasted->typeSpecifiedDescriptor);
    
		ledChannelReset(manager);    
}

static void ledChannelStartRecording(void* base, int32_t timeMS)
{
    DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(!baseCasted->isEnabled)
				return;
		
		if(baseCasted->type != DEVICE_TYPE_LED)
				// error
				return;
    LedChannelManager * manager = (LedChannelManager*) (baseCasted->typeSpecifiedDescriptor);
    
		if(manager->base.status != DEVICE_STATUS_WAIT)
        // device is already running or not prepared
        return;
		
		if(setChannelPWM_Value(manager->channel, 0))
		{
			// hardware error
			ledChannelReset(manager);
			manager->base.status = DEVICE_STATUS_ERROR;
			return;
		}
		ledHardwareInit(manager->channel);
		manager->beginningTime = timeMS;
    manager->base.status = DEVICE_STATUS_RECORDING;
}


static void ledChannelHandle(void * base, int32_t timeMS)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(!baseCasted->isEnabled)
				return;
		
		if(baseCasted->type != DEVICE_TYPE_LED)
				// error
				return;
    LedChannelManager * manager = (LedChannelManager*) (baseCasted->typeSpecifiedDescriptor);
    
		if(manager->base.status != DEVICE_STATUS_RECORDING)
				// not allowed
				return;
	
		if(isnan(manager->next.timeInSeconds))
		{
			// error
			ledChannelReset(manager);
			baseCasted->status = DEVICE_STATUS_ERROR;
			return;
		}
		float timeFromBeginning = (float)(timeMS - manager->beginningTime) / 1000;
		if(manager->next.timeInSeconds < timeFromBeginning)
		{
			manager->current = manager->next;
			FileParsingResult res = getLedFilePoint(&(manager->buffer), &(manager->next));
			if(res != FILE_PARSING_RESULT_SUCCESS)
			{
				ledChannelReset(manager);
				if(res == FILE_PARSING_RESULT_EOF)
				{
					baseCasted->status = DEVICE_STATUS_FREE;
				}
				else
				{
					baseCasted->status = DEVICE_STATUS_ERROR;
				}
				return;
			}
		}
		if(isnan(manager->current.timeInSeconds))
		{
			// it's not a time yet
			return;
		}
		
		// linear interpolation
		float valueRange = manager->next.value - manager->current.value;
		float timeRange = manager->next.timeInSeconds - manager->current.timeInSeconds;
		float channelValue = manager->current.value + (timeFromBeginning - manager->current.timeInSeconds) * valueRange / timeRange;
		if(setChannelPWM_Value(manager->channel, (uint32_t)(channelValue * 255)))
		{
				// hardware error
				ledChannelReset(manager);
				manager->base.status = DEVICE_STATUS_ERROR;
				return;
		}			
}

void ledChannelManagerInit(LedChannelManager* this, LedChannel channel)
{
		this->base.status 									= DEVICE_STATUS_FREE;
		this->base.typeSpecifiedDescriptor	= this;
		this->base.type 										= DEVICE_TYPE_LED;
		this->base.next 										= NULL;
	
		this->base.link 				= ledChannelLinkFile;
		this->base.handle 			= ledChannelHandle;
		this->base.start 				= ledChannelStartRecording;
		this->base.stop 				= ledChannelEndRecording;
		this->base.isEnabled 		= false;
		this->base.hardwareName = NULL;
	
		this->channel = channel;
		this->isFileLinked = false;
		streamFileBufferInit(&this->buffer, this->bufferData, LED_CHANNEL_BUFFER_SIZE);
		ledChannelReset(this);
}

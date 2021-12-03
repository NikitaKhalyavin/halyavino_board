#include "executing_manager.h"

#include <stddef.h>
#include <string.h>

#include "platform_defines.h"
#include "stream_file_buffer.h"
#include "fs_api.h"
#include "status_led.h"
#include "data_queue.h"

#include "led/led_channel_manager.h"
#include "speaker/sound_manager.h"
#include "gpio/gpio_channel_manager.h"

//------------GLOBAL DEFINES------------------------

static ExecutingStatus globalStatus;
DeviceManagerBase* globalDeviceList;

#define MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH 16
uint8_t keywordBuffer[MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH];
#define FILE_READING_BUFFER_SIZE 32
uint8_t fileReadingBufferData[FILE_READING_BUFFER_SIZE];
StreamFileBuffer fileReadingBuffer;

typedef enum {CONFIG_PARAM_TYPE_ENABLED,	CONFIG_PARAM_TYPE_MODE, 
		CONFIG_PARAM_TYPE_BUTTON_ACTIVE_STATE,	CONFIG_PARAM_TYPE_FILENAME,																	
		CONFIG_PARAM_TYPE_ERROR} ConfigParamType;

typedef struct 
{
		ConfigParamType type;
		union
		{
				bool isEnabled;
				DeviceType mode;
				
				bool activeStateHigh;
			
				// for button_filename - result is in the buffer
				const char* fileName;
		} value;
} ConfigParamDescriptor;

static void executingManagerRunScript(const char* scriptName);

static bool applyNextConfigString(StreamFileBuffer* source);
static void applyNextScriptString(StreamFileBuffer* source);
ConfigParamDescriptor getNextParam(StreamFileBuffer* source, uint8_t* buffer, uint32_t maxSize);
static bool blockingReadFromBuffer(StreamFileBuffer* source, uint8_t* dst, FileSize size);
static int getNextKeyword(StreamFileBuffer* source, uint8_t* dst, uint32_t maxSize);
static void skipLineRemaining(StreamFileBuffer* source);

static bool isLastReadedKeywordLastInTheString = false;
static bool isLastReadedKeywordLastInTheFile = false;
static bool isLastReadedParamType = false;

#define CONFIG_ERROR_STATUS_LED_FREQ_HZ 4
static int32_t lastStatusLedStateChangingTimeMS;

static void changeStateToListening(int32_t timeMS);
static void changeStateToRecording(int32_t timeMS);
static void changeStateToPreparing(void);

DataQueue cmdQueue;
#define CMD_QUEUE_SIZE 5
static uint8_t cmdQueueBuffer[CMD_QUEUE_SIZE * sizeof(const char*)];

// CONFIG PARAM TYPES
static const char PARAM_TYPE_ENABLED_STRING[] 				= "ENABLED";
static const char PARAM_TYPE_MODE_STRING[] 						= "MODE";
static const char PARAM_TYPE_BTN_ACT_STATE_STRING[] 	= "BTN_ACT_STATE";
static const char PARAM_TYPE_FILENAME_STRING[] 		= "FILENAME";

// CONFIG PARAM VALUES
static const char PARAM_VALUE_TRUE_STRING[] 					= "TRUE";
static const char PARAM_VALUE_FALSE_STRING[] 					= "FALSE";

static const char PARAM_VALUE_SPEAKER_STRING[] 				= "SPEAKER";
static const char PARAM_VALUE_LED_STRING[] 						= "LED";
static const char PARAM_VALUE_BUTTON_STRING[] 				= "BUTTON";

static const char PARAM_VALUE_HIGH_STRING[] 					= "HIGH";
static const char PARAM_VALUE_LOW_STRING[] 					  = "LOW";
//-----------GLOBAL DEFINES END---------------------

//----------DEVICES DEFINING-----------------------

//nonstatic, because it's using in interrupt
SpeakerManager speaker;
const char speakerHardwareName[] = "SPEAKER";

static LedChannelManager ledChannels[LED_CHANNELS_COUNT];
const char ledHardwareNames[LED_CHANNELS_COUNT][5] = 
{
"LED1",
"LED2",
"LED3",
"LED4",
"LED5",
"LED6"
};
static GPIO_ChannelManager GPIO_Channels[GPIO_CHANNELS_COUNT];
const char gpioHardwareNames[GPIO_CHANNELS_COUNT][6] = 
{
"GPIO1",
"GPIO2",
"GPIO3",
"GPIO4",
"GPIO5",
"GPIO6",
"GPIO7",
"GPIO8"
};

//-----------DEVICES DEFINING END--------------------


void executingManagerInit(void)
{
		streamFileBufferInit(&fileReadingBuffer, fileReadingBufferData, FILE_READING_BUFFER_SIZE);
		dataQueueInit(&cmdQueue, CMD_QUEUE_SIZE, cmdQueueBuffer, sizeof(const char*));
		speakerManagerInit(&speaker);
		speaker.base.hardwareName = speakerHardwareName;
		globalDeviceList = &speaker.base;
		DeviceManagerBase* current = globalDeviceList;
		for(int channelNumber = 0; channelNumber < LED_CHANNELS_COUNT; channelNumber++)
		{
				ledChannelManagerInit(&ledChannels[channelNumber], channelNumber);
				ledChannels[channelNumber].base.hardwareName = ledHardwareNames[channelNumber];
				current->next = &ledChannels[channelNumber].base;
				current = current->next;
		}
		for(int channelNumber = 0; channelNumber < GPIO_CHANNELS_COUNT; channelNumber++)
		{
				GPIO_ChannelManagerInit(&GPIO_Channels[channelNumber], channelNumber);
				GPIO_Channels[channelNumber].base.hardwareName = gpioHardwareNames[channelNumber];
				current->next = &GPIO_Channels[channelNumber].base;
				current = current->next;
		}
}


void executingManagerReconfigBoard(int32_t timeMS)
{
		setStatusLedOff();
		lastStatusLedStateChangingTimeMS = timeMS;
		
		globalStatus = EXECUTING_STATUS_NOT_CONFIGURED;
		FS_ApiResult res = linkFileToBuffer(&fileReadingBuffer, "config.txt", 
									FILE_TRANSFER_DIRECTION_GET, FILE_DEVICE_STORAGE);
		if(res != FS_API_RESULT_SUCCESS)
				return;
		
		isLastReadedKeywordLastInTheFile = false;
		while(!isLastReadedKeywordLastInTheFile)
		{
				if(applyNextConfigString(&fileReadingBuffer))
				{
						unlinkFileFromBuffer(&fileReadingBuffer);
						return;
				}
		}
		
		unlinkFileFromBuffer(&fileReadingBuffer);
		changeStateToListening(timeMS);
}

void executingManagerGetScriptCmd(const char* fileName)
{
		DataQueueEnqueueResult res = cmdQueue.tryToEnqueue(&cmdQueue, &fileName);
}

static void executingManagerRunScript(const char* scriptName)
{
		FS_ApiResult res = linkFileToBuffer(&fileReadingBuffer, scriptName, 
								FILE_TRANSFER_DIRECTION_GET, FILE_DEVICE_STORAGE);
		if(res != FS_API_RESULT_SUCCESS)
				return;
		
		changeStateToPreparing();
		
		isLastReadedKeywordLastInTheFile = false;
		while(!isLastReadedKeywordLastInTheFile)
		{
				applyNextScriptString(&fileReadingBuffer);
		}
		unlinkFileFromBuffer(&fileReadingBuffer);
}

void executingManagerHandle(int32_t timeMS)
{
		if(globalStatus == EXECUTING_STATUS_NOT_CONFIGURED)
		{
				if(timeMS - lastStatusLedStateChangingTimeMS > 1000 / CONFIG_ERROR_STATUS_LED_FREQ_HZ)
				{
						toggleStatusLed();
						lastStatusLedStateChangingTimeMS = timeMS;
				}
				return;
		}
		
		if(globalStatus == EXECUTING_STATUS_PREPARING)
		{
				//after script parsing
				changeStateToRecording(timeMS);
		}
		
		if(globalStatus == EXECUTING_STATUS_IDLE)
		{
				// only buttons can work here
				DeviceManagerBase* current = globalDeviceList;
				while(current != NULL)
				{
						bool isDeviceTypeEnabled = current->type == DEVICE_TYPE_BUTTON;
						if(isDeviceTypeEnabled && (current->isEnabled) && (current->status == DEVICE_STATUS_RECORDING))
								current->handle(current, timeMS);
						current = current->next;
				}
				const char* fileName;
				DataQueueDequeueResult res = cmdQueue.tryToDequeue(&cmdQueue, &fileName);
				if(res == DATA_QUEUE_DEQUEUE_RESULT_SUCCESS)
				{
						executingManagerRunScript(fileName);
				}
		}
		
		if(globalStatus == EXECUTING_STATUS_RECORDING)
		{
				DeviceManagerBase* current = globalDeviceList;
				uint32_t activeDevicesNumber = 0;
				while(current != NULL)
				{
						bool isDeviceTypeEnabled = current->type == DEVICE_TYPE_LED || current->type == DEVICE_TYPE_SPEAKER;
						if(isDeviceTypeEnabled && (current->isEnabled) && (current->status == DEVICE_STATUS_RECORDING))
						{
								current->handle(current, timeMS);
								activeDevicesNumber++;
						}
						current = current->next;
				}
				if(activeDevicesNumber == 0)
				{
						changeStateToListening(timeMS);
				}
		}
}


static void changeStateToListening(int32_t timeMS)
{
		DeviceManagerBase* current = globalDeviceList;
		while(current != NULL)
		{
				bool isDeviceTypeAllowed = (current->type == DEVICE_TYPE_BUTTON);
				if(isDeviceTypeAllowed && current->isEnabled)
				{
						current->start(current, timeMS);
				}
				else
				{
						if(current->isEnabled)
								current->stop(current);
				}
				current = current->next;
		}
		globalStatus = EXECUTING_STATUS_IDLE;
}

static void changeStateToPreparing(void)
{
		// just stop all buttons
		DeviceManagerBase* current = globalDeviceList;
		while(current != NULL)
		{
				bool isDeviceTypeNotAllowed = (current->type == DEVICE_TYPE_LED) || 
							(current->type == DEVICE_TYPE_SPEAKER);
				if(isDeviceTypeNotAllowed)
				{
						if((current->status == DEVICE_STATUS_RECORDING) && current->isEnabled)
								current->stop(current);
				}
				current = current->next;
		}
		globalStatus = EXECUTING_STATUS_PREPARING;
}

static void changeStateToRecording(int32_t timeMS)
{
		DeviceManagerBase* current = globalDeviceList;
		while(current != NULL)
		{
				bool isDeviceTypeAllowed = (current->type == DEVICE_TYPE_LED) || 
							(current->type == DEVICE_TYPE_SPEAKER);
				if(isDeviceTypeAllowed && current->isEnabled)
				{
						if(current->status == DEVICE_STATUS_WAIT)
								current->start(current, timeMS);
				}
				else
				{
						if(current->isEnabled)
								current->stop(current);
				}
				current = current->next;
		}
		globalStatus = EXECUTING_STATUS_RECORDING;
}


//-------------------------------------------------------------------
//         CONFIG   FILE   PARSER
//-------------------------------------------------------------------
static bool applyNextConfigString(StreamFileBuffer* source)
{
		// config file string format: (separator - ' ')
		// DEVICE_NAME ENABLED/DISABLED(default) MODE(for GPIO only, default DISABLED), fileName(for buttons only)
	
		// get DEVICE_NAME
		int deviceNameSize = getNextKeyword(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
		if(deviceNameSize < 4)
		{
				//error: there is no devices with such short name
				skipLineRemaining(source);
				return true;
		}
		
		DeviceManagerBase* current = globalDeviceList;
		while(current != NULL)
		{
				if(strlen(current->hardwareName) == deviceNameSize)
				{
						int compRes = strncmp((void*)keywordBuffer, current->hardwareName, deviceNameSize);
						if(compRes == 0)
						{
								// strings are equals
								break;
						}
				}
				current = current->next;
		}
		if(current == NULL)
		{
				// error: unknown device
				skipLineRemaining(source);
				return true;
		}
		
		DeviceType hardwareType = current->type;
		
		switch(hardwareType)
		{
				case DEVICE_TYPE_LED:
				{
						bool isEnabled = false;
						while(!isLastReadedKeywordLastInTheString)
						{
								ConfigParamDescriptor param = getNextParam(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
								switch(param.type)
								{
										case CONFIG_PARAM_TYPE_ENABLED:
												isEnabled = param.value.isEnabled;
												break;
										
										case CONFIG_PARAM_TYPE_MODE:
												if(param.value.mode != DEVICE_TYPE_LED)
												{
														// error: incompatible mode
														skipLineRemaining(source);
														return true;
												}
												break;
										default:
												// error: param type is not allowed
												skipLineRemaining(source);
												return true;
								}
						}
						current->isEnabled = isEnabled;
						return false;
				}
				case DEVICE_TYPE_SPEAKER:
				{
						bool isEnabled = false;
						while(!isLastReadedKeywordLastInTheString)
						{
								ConfigParamDescriptor param = getNextParam(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
								switch(param.type)
								{
										case CONFIG_PARAM_TYPE_ENABLED:
												isEnabled = param.value.isEnabled;
												break;
										
										case CONFIG_PARAM_TYPE_MODE:
												if(param.value.mode != DEVICE_TYPE_SPEAKER)
												{
														// error: incompatible mode
														skipLineRemaining(source);
														return true;
												}
												break;
										default:
												// error: param type is not allowed
												skipLineRemaining(source);
												return true;
								}
						}
						current->isEnabled = isEnabled;
						return false;
				}
				case DEVICE_TYPE_GPIO_DISABLED:
				{
						bool isEnabled = false;
						DeviceType type = DEVICE_TYPE_GPIO_DISABLED;
						GPIO_State activeState;
						char fileName[MAX_SCRIPT_FILENAME_LENGTH];
						while(!isLastReadedKeywordLastInTheString)
						{
								ConfigParamDescriptor param = getNextParam(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
								switch(param.type)
								{
										case CONFIG_PARAM_TYPE_ENABLED:
												{
														isEnabled = param.value.isEnabled;
														break;
												}
										
										case CONFIG_PARAM_TYPE_MODE:
												{
														if((param.value.mode == DEVICE_TYPE_BUTTON) || (param.value.mode == DEVICE_TYPE_GPIO_DISABLED))
														{
																type = param.value.mode;
														}
														else
														{
																// error: incompatible mode
																skipLineRemaining(source);
																return true;
														}
														break;
												}
										
										case CONFIG_PARAM_TYPE_BUTTON_ACTIVE_STATE:
												{
														if(type != DEVICE_TYPE_BUTTON)
														{
																// error: set type first
																skipLineRemaining(source);
																return true;
														}
														if(param.value.activeStateHigh)
																activeState = GPIO_STATE_HIGH;
														else
																activeState = GPIO_STATE_LOW;
														break;
												}
										
										case CONFIG_PARAM_TYPE_FILENAME:
												{
														if(type != DEVICE_TYPE_BUTTON)
														{
																// error: set type first
																skipLineRemaining(source);
																return true;
														}
														uint32_t nameLen = strlen(param.value.fileName) + 1; // +1 is '\0'
														if(nameLen >= MAX_SCRIPT_FILENAME_LENGTH)
														{
																// error: too long name
																skipLineRemaining(source);
																return true;
														}
														
														memcpy(fileName, param.value.fileName, nameLen);
														break;
												}
												
										default:
												{
														// error: param type is not allowed
														skipLineRemaining(source);
														return true;
												}
								}
						}

						if(isEnabled)
						{
						current->isEnabled = isEnabled;
						GPIO_ChannelManager* manager = current->typeSpecifiedDescriptor;
						switch(type)
						{
								case DEVICE_TYPE_BUTTON:
								{
										GPIO_ChannelManagerConfigAsButton(manager, fileName);
										manager->modeSpecified.button.desc.activeState = activeState;
										break;
								}
								default:
										// not allowed
										return true;
						}
						return false;
						}
				}
				default:
				{
						// error: not allowed pre-config type
						skipLineRemaining(source);
						return true;
				}
		}
}

static void applyNextScriptString(StreamFileBuffer* source)
{
		// config file string format: (separator - ' ')
		// DEVICE_NAME ENABLED/DISABLED(default) MODE(for GPIO only, default DISABLED), fileName(for buttons only)
	
		// get DEVICE_NAME
		int deviceNameSize = getNextKeyword(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
		if(deviceNameSize < 4)
		{
				//error: there is no devices with such short name
				skipLineRemaining(source);
				return;
		}
		
		DeviceManagerBase* current = globalDeviceList;
		while(current != NULL)
		{
				if(strlen(current->hardwareName) == deviceNameSize)
				{
						int compRes = strncmp((void*)keywordBuffer, current->hardwareName, deviceNameSize);
						if(compRes == 0)
						{
								// strings are equals
								break;
						}
				}
				current = current->next;
		}
		if(current == NULL)
		{
				// error: unknown device
				skipLineRemaining(source);
				return;
		}
		
		bool isDeviceInput = current->type == DEVICE_TYPE_BUTTON;
		if(isDeviceInput || (!current->isEnabled))
		{
				// error: unable to link to this device
				skipLineRemaining(source);
				return;
		}
						
		while(!isLastReadedKeywordLastInTheString)
		{
				ConfigParamDescriptor param = getNextParam(source, keywordBuffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
				switch(param.type)
				{
						case CONFIG_PARAM_TYPE_FILENAME:
								current->link(current, param.value.fileName);
								break;
						
						default:
								// error: param type is not allowed
								skipLineRemaining(source);
								return;
				}
		}
			
}

ConfigParamDescriptor getNextParam(StreamFileBuffer* source, uint8_t* buffer, uint32_t maxSize)
{
		ConfigParamDescriptor result;
	
		result.type = CONFIG_PARAM_TYPE_ERROR;
		
		// read paramType
		int paramTypeLength = getNextKeyword(source, buffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
		if((!isLastReadedParamType) || (paramTypeLength < 3))
		{
				return result;
		}
		
		if((strncmp((void*)buffer, PARAM_TYPE_ENABLED_STRING, paramTypeLength) == 0) &&
				(strlen(PARAM_TYPE_ENABLED_STRING) == paramTypeLength))
		{
				result.type = CONFIG_PARAM_TYPE_ENABLED;
		}
		if((strncmp((void*)buffer, PARAM_TYPE_MODE_STRING, paramTypeLength) == 0) &&
				(strlen(PARAM_TYPE_MODE_STRING) == paramTypeLength))
		{
				result.type = CONFIG_PARAM_TYPE_MODE;
		}
		if((strncmp((void*)buffer, PARAM_TYPE_BTN_ACT_STATE_STRING, paramTypeLength) == 0) &&
				(strlen(PARAM_TYPE_BTN_ACT_STATE_STRING) == paramTypeLength))
		{
				result.type = CONFIG_PARAM_TYPE_BUTTON_ACTIVE_STATE;
		}
		if((strncmp((void*)buffer, PARAM_TYPE_FILENAME_STRING, paramTypeLength) == 0) &&
				(strlen(PARAM_TYPE_FILENAME_STRING) == paramTypeLength))
		{
				result.type = CONFIG_PARAM_TYPE_FILENAME;
		}
		
		if(result.type == CONFIG_PARAM_TYPE_ERROR)
				return result;
		
		// read value
		int valueLength = getNextKeyword(source, buffer, MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH);
		switch(result.type)
		{
				case CONFIG_PARAM_TYPE_ENABLED:
				{
						if((strncmp((void*)buffer, PARAM_VALUE_TRUE_STRING, valueLength) == 0) &&
									(strlen(PARAM_VALUE_TRUE_STRING) == valueLength))
								result.value.isEnabled = true;		
						else
						{
								if((strncmp((void*)buffer, PARAM_VALUE_FALSE_STRING, valueLength) == 0) &&
										(strlen(PARAM_VALUE_FALSE_STRING) == valueLength))
										result.value.isEnabled = false;		
								else
										// unknown value
										result.type = CONFIG_PARAM_TYPE_ERROR;
						}
						return result;
				}
				case CONFIG_PARAM_TYPE_MODE:
				{
						if((strncmp((void*)buffer, PARAM_VALUE_SPEAKER_STRING, valueLength) == 0) &&
									(strlen(PARAM_VALUE_SPEAKER_STRING) == valueLength))
								result.value.mode = DEVICE_TYPE_SPEAKER;		
						else
						{
								if((strncmp((void*)buffer, PARAM_VALUE_LED_STRING, valueLength) == 0) &&
										(strlen(PARAM_VALUE_LED_STRING) == valueLength))
										result.value.mode = DEVICE_TYPE_LED;		
								else
								{
									
										if((strncmp((void*)buffer, PARAM_VALUE_BUTTON_STRING, valueLength) == 0) &&
												(strlen(PARAM_VALUE_BUTTON_STRING) == valueLength))
												result.value.mode = DEVICE_TYPE_BUTTON;	
										else
										{
												// unknown or not allowed value
												result.type = CONFIG_PARAM_TYPE_ERROR;
										}
								}
						}
						return result;
				}
				case CONFIG_PARAM_TYPE_BUTTON_ACTIVE_STATE:
				{
						if((strncmp((void*)buffer, PARAM_VALUE_HIGH_STRING, valueLength) == 0) &&
									(strlen(PARAM_VALUE_HIGH_STRING) == valueLength))
								result.value.activeStateHigh = true;
						else
						{
								if((strncmp((void*)buffer, PARAM_VALUE_LOW_STRING, valueLength) == 0) &&
										(strlen(PARAM_VALUE_LOW_STRING) == valueLength))
										result.value.activeStateHigh = false;
								else
										// unknown value
										result.type = CONFIG_PARAM_TYPE_ERROR;
						}
						return result;
				}
				case CONFIG_PARAM_TYPE_FILENAME:
				{
						if((valueLength == MAX_CONFIG_AND_SCRIPT_KEYWORD_LENGTH) || 
								(valueLength >= MAX_SCRIPT_FILENAME_LENGTH))
						{
								result.type = CONFIG_PARAM_TYPE_ERROR;
						}
						else
						{
								buffer[valueLength] = '\0';
								result.value.fileName = (void*)buffer;
						}
						return result;
				}
				default:
				{
						result.type = CONFIG_PARAM_TYPE_ERROR;
						return result;
				}
		}
}

static void skipLineRemaining(StreamFileBuffer* source)
{
		if(isLastReadedKeywordLastInTheString)
				return;
		
		isLastReadedKeywordLastInTheString = true;
		char readed;
		// skip all separators
		do
		{
				if(blockingReadFromBuffer(source, (void*)&readed, 1))
				{
						isLastReadedKeywordLastInTheFile = true;
						return;
				}
		}while ((readed != '\n') && (readed != '\0'));
		if(readed == '\0')
				isLastReadedKeywordLastInTheFile = true;
}

static int getNextKeyword(StreamFileBuffer* source, uint8_t* dst, uint32_t maxSize)
{
		int cursor = 0;
		char readed;
		isLastReadedKeywordLastInTheString = false;
		isLastReadedParamType = false;
		// skip all separators
		do
		{
				if(blockingReadFromBuffer(source, (void*)&readed, 1))
				{
						isLastReadedKeywordLastInTheFile = true;
						isLastReadedKeywordLastInTheString = true;
						return 0;
				}
		}while (readed == ' ');
		
		while ((readed != ' ') && (readed != '\n') && (readed != '\0') && (readed != '='))
		{
				if(readed != '\r')
				{
						dst[cursor] = readed;
						cursor++;
				}
				if(blockingReadFromBuffer(source, (void*)&readed, 1))
				{
						isLastReadedKeywordLastInTheFile = true;
						isLastReadedKeywordLastInTheString = true;
						return cursor;
				}
		}
		if(readed == '=')
		{
				isLastReadedParamType = true;
				return cursor;
		}
		if(readed != ' ')
				isLastReadedKeywordLastInTheString = true;
		if(readed == '\0')
				isLastReadedKeywordLastInTheFile = true;
		return cursor;
}

static bool blockingReadFromBuffer(StreamFileBuffer* source, uint8_t* dst, FileSize size)
{
		FileSize readedBytesNumber = 0;
		while(readedBytesNumber < size)
		{
				readedBytesNumber += source->read(source, dst + readedBytesNumber, size - readedBytesNumber);
				if(readedBytesNumber < size)
				{	
						executingManagerThreadDelay();
						switch(source->status)
						{
								case STREAM_BUFFER_STATUS_FILE_END:
										if(source->size > 0)
												continue;
										return true;
								case STREAM_BUFFER_STATUS_IN_PROCESS:
										continue;
								default:
										// error
										return true;
						}
				}
		}
		return false;
}

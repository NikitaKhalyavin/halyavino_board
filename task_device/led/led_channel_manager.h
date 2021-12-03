#ifndef LED_CHANNEL_MANAGER_H
#define LED_CHANNEL_MANAGER_H

#include "device_manager.h"
#include "stream_file_buffer.h"
#include "led_port.h"
#include "led_file_parser.h"

#define LED_CHANNEL_BUFFER_SIZE 32
typedef struct
{
    DeviceManagerBase base;
    
    StreamFileBuffer buffer;
		bool isFileLinked;
		uint8_t bufferData[LED_CHANNEL_BUFFER_SIZE];
    LedChannel channel;
	
		//private data
		uint32_t beginningTime;
		LedFilePoint current;
		LedFilePoint next;
    uint32_t pointNumber;
} LedChannelManager;

void ledChannelManagerInit(LedChannelManager* this, LedChannel channel);


#endif

#ifndef GPIO_CHANNEL_MANAGER_H
#define GPIO_CHANNEL_MANAGER_H

#include "stream_file_buffer.h"
#include "device_manager.h"
#include "gpio/gpio_port.h"

#include "gpio/servo_channel_descriptor.h"
#include "gpio/button_channel_descriptor.h"
#include <stdbool.h>

//--------------------servo mode defines-----------------------
#define SERVO_MODE_BUFFER_SIZE 32
typedef struct
{
		ServoChannelDescriptor desc;
		StreamFileBuffer buffer;
		uint8_t bufferData[SERVO_MODE_BUFFER_SIZE];
} ServoModeData;
//-------------------servo mode defines end--------------------

//--------------------button mode defines-----------------------
#define MAX_SCRIPT_FILENAME_LENGTH 32
typedef struct
{
		ButtonChannelDescriptor desc;
		char scriptFileName[MAX_SCRIPT_FILENAME_LENGTH];
} ButtonModeData;
//-------------------button mode defines end--------------------

typedef struct
{
		DeviceManagerBase base;
    GPIO_Channel channel;
    union
    {
        ButtonModeData button;
        ServoModeData servo;
    } modeSpecified;
    
}GPIO_ChannelManager;

void GPIO_ChannelManagerInit(GPIO_ChannelManager* manager, GPIO_Channel channel);
void GPIO_ChannelManagerConfigAsButton(GPIO_ChannelManager* manager, const char* scriptName);
// TO-DO: void GPIO_ChannelManagerConfigAsServo(GPIO_ChannelManager* manager);

#endif

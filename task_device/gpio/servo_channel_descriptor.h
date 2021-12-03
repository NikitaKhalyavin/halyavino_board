#ifndef SERVO_CHANNEL_DESCRIPTOR_H_
#define SERVO_CHANNEL_DESCRIPTOR_H_

#include <stdbool.h>
#include <stdint.h>

#include "gpio_port.h"

typedef struct
{
	void* next;
	bool isAllowed;
	int32_t angle; 
	int32_t minAngle;
	int32_t maxAngle;
	uint32_t minPulseTime;
	uint32_t maxPulseTime;
	
	GPIO_Channel channel;
}ServoChannelDescriptor;

//list has to be circular
void servoChannelDescriptorProcess(ServoChannelDescriptor** list);
void servoChannelDescriptorFinishProcess(ServoChannelDescriptor** list);

#endif // SERVO_CHANNEL_DESCRIPTOR_H_

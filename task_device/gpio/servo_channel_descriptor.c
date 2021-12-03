#include "servo_channel_descriptor.h"
#include "servo_port.h"
#include "gpio_port.h"
#include <stddef.h>

void servoChannelDescriptorProcess(ServoChannelDescriptor** list)
{
	if(list == NULL)
		return;
	
	ServoChannelDescriptor* current = *list;
	if(current == NULL)
		return;
	
	*list = current->next;
	
	current = *list;
	if(current == NULL)
		return;
	
	
	if(current->isAllowed)
	{
		setGPIO_State(current->channel, GPIO_STATE_HIGH);
		(current->callbackArgs);
		int32_t angleWidth = current->maxAngle - current->minAngle;
		uint32_t timeWidth = current->maxPulseTime - current->minPulseTime;
		
		uint32_t timeMCS = current->minPulseTime + 
			(current->angle - current->minAngle) * timeWidth / angleWidth;
		
		bool res = servoSetTimer(timeMCS);
		if(res)
			servoChannelDescriptorFinishProcess(list);
	}
	else
		servoChannelDescriptorFinishProcess(list);
}

void servoChannelDescriptorFinishProcess(ServoChannelDescriptor** list)
{
	if(list == NULL)
		return;
	
	ServoChannelDescriptor* current = *list;
	if(current == NULL)
		return;
	
	if(current->isAllowed)
	{
		setGPIO_State(current->channel, GPIO_STATE_LOW);
		servoResetTimer();
	}
}

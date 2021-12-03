#include "button_channel_descriptor.h"
#include "gpio_channel_manager.h"
#include "executing_manager.h"
#include <limits.h>

#define BUTTON_ELAPSING_TIME_MS 100

static void buttonStart(void* base, int32_t timeMS);
static void buttonHandle(void* base, int32_t time);
static void buttonLink(void* base, const char* fileName);
static void buttonStop(void* base);

void initDeviceAsButton(DeviceManagerBase* base)
{
		base->type = DEVICE_TYPE_BUTTON;
		base->handle = buttonHandle;
		base->start = buttonStart;
		base->link = buttonLink;	
		base->stop = buttonStop;
		base->status = DEVICE_STATUS_WAIT;
}

static void buttonStart(void* base, int32_t timeMS)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_BUTTON)
				//error
				return;
		
		GPIO_ChannelManager* manager = baseCasted->typeSpecifiedDescriptor;
		ButtonChannelDescriptor* button = &(manager->modeSpecified.button.desc);
		
		button->lastState = BUTTON_STATE_RELAXED;
		button->lastChangeTime = timeMS - BUTTON_ELAPSING_TIME_MS;
		manager->base.status = DEVICE_STATUS_RECORDING;
}

static void buttonHandle(void* base, int32_t time)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_BUTTON)
				//error
				return;
		
		GPIO_ChannelManager* manager = baseCasted->typeSpecifiedDescriptor;
		
		if(manager->base.status != DEVICE_STATUS_RECORDING)
				return;
		
		ButtonChannelDescriptor* button = &(manager->modeSpecified.button.desc);
		
		int32_t timeSinceLastChanging = time - button->lastChangeTime;
		if(timeSinceLastChanging < BUTTON_ELAPSING_TIME_MS)
				return;
		
		if(timeSinceLastChanging > INT32_MAX / 2)
		{
				// for prevent button disabling because of the overflow
				timeSinceLastChanging = time - BUTTON_ELAPSING_TIME_MS;
		}
		
		GPIO_State channelState = getGPIO_ChannelState(button->channel);
		
		ButtonState currentState;
		
		if(channelState == button->activeState)
				currentState = BUTTON_STATE_PRESSED;
		else
				currentState = BUTTON_STATE_RELAXED;
		
		if(currentState != button->lastState)
		{
				button->lastChangeTime = time;
				button->lastState = currentState;
				
				if(currentState == BUTTON_STATE_PRESSED)
				{
						// TO-DO: call sisEvent function
						executingManagerGetScriptCmd(manager->modeSpecified.button.scriptFileName);
				}
		}
}


static void buttonLink(void* base, const char* fileName)
{
		// dummy
		return;
}
static void buttonStop(void* base)
{
		DeviceManagerBase* baseCasted = (DeviceManagerBase*)base;
		if(baseCasted->type != DEVICE_TYPE_BUTTON)
				//error
				return;
		
		GPIO_ChannelManager* manager = baseCasted->typeSpecifiedDescriptor;
		manager->base.status = DEVICE_STATUS_WAIT;
}

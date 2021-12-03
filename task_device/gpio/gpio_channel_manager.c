#include "gpio/gpio_channel_manager.h"
#include "gpio/gpio_port.h"
#include <stdlib.h>
#include <string.h>


void GPIO_ChannelManagerInit(GPIO_ChannelManager* this, GPIO_Channel channel)
{
    this->channel = channel;
	
		this->base.status 									= DEVICE_STATUS_FREE;
		this->base.typeSpecifiedDescriptor	= this;
		this->base.type 										= DEVICE_TYPE_GPIO_DISABLED;
		this->base.next 										= NULL;
		this->base.isEnabled 								= false;
		this->base.hardwareName 						= NULL;
	
		this->base.link 	= NULL;
		this->base.handle = NULL;
		this->base.start 	= NULL;
		this->base.stop 	= NULL;
}


void GPIO_ChannelManagerConfigAsButton(GPIO_ChannelManager* manager, const char* scriptName)
{
		initDeviceAsButton(&manager->base);
		manager->modeSpecified.button.desc.activeState = GPIO_STATE_LOW;
		manager->modeSpecified.button.desc.channel = manager->channel;
		strncpy(manager->modeSpecified.button.scriptFileName, scriptName, MAX_SCRIPT_FILENAME_LENGTH);
}

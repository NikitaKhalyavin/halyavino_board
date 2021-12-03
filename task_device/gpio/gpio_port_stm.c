#include "gpio/gpio_port.h"
#include "main.h"

typedef struct 
{
	GPIO_TypeDef* GPIO_Port;
	uint16_t GPIO_Pin;
}GPIO_ChannelDescriptor;

static __inline GPIO_ChannelDescriptor getGPIO_ChannelDescriptor(GPIO_Channel channelNumber)
{
		GPIO_ChannelDescriptor result;
		result.GPIO_Port = GPIOA;
		switch(channelNumber)
		{
        case 0:
            result.GPIO_Pin = GPIO1_Pin;
						break;
        case 1:
            result.GPIO_Pin = GPIO2_Pin;
						break;
        case 2:
            result.GPIO_Pin = GPIO3_Pin;
						break;
        case 3:
            result.GPIO_Pin = GPIO4_Pin;
						break;
        case 4:
            result.GPIO_Pin = GPIO5_Pin;
						break;
        case 5:
            result.GPIO_Pin = GPIO6_Pin;
						break;
        case 6:
            result.GPIO_Pin = GPIO7_Pin;
						break;
        case 7:
            result.GPIO_Pin = GPIO8_Pin;
						break;
		}
		return result;
}

static __inline void setHardwareState(GPIO_ChannelDescriptor port, GPIO_State state)
{		
		if(state == GPIO_STATE_HIGH)
				HAL_GPIO_WritePin(port.GPIO_Port, port.GPIO_Pin, GPIO_PIN_SET);
		else
				HAL_GPIO_WritePin(port.GPIO_Port, port.GPIO_Pin, GPIO_PIN_RESET);
}

static __inline GPIO_State getHardwareState(GPIO_ChannelDescriptor port)
{		
		if(HAL_GPIO_ReadPin(port.GPIO_Port, port.GPIO_Pin) == GPIO_PIN_SET)
						return GPIO_STATE_HIGH;
		else
				return GPIO_STATE_LOW;
}

void setGPIO_ChannelState(GPIO_Channel gpio, GPIO_State state)
{
		GPIO_ChannelDescriptor desc = getGPIO_ChannelDescriptor(gpio);
		setHardwareState(desc, state);
}

GPIO_State getGPIO_ChannelState(GPIO_Channel gpio)
{
		GPIO_ChannelDescriptor desc = getGPIO_ChannelDescriptor(gpio);
		return  getHardwareState(desc);
}

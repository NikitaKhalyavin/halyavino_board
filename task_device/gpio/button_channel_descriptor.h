#ifndef BUTTON_CHANNEL_DESCRIPTOR_H_
#define BUTTON_CHANNEL_DESCRIPTOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "device_manager.h"
#include "gpio/gpio_port.h"

typedef enum {BUTTON_STATE_RELAXED, BUTTON_STATE_PRESSED} ButtonState;
/*
* button working algorithm:
* 1) button is in RELAXED state
* 2) button changed state to PRESSED (in handle function) when button is actually pressed
* 3) call sisEvent and set lastChangeTime - button is inactive for a while
* 4) button cannot call sisEvent again before reaching RELAXED state
* 5) after reaching RELAXING state lastChangeTime is updated again - button is inactive for a while
*/

typedef struct
{
		GPIO_Channel channel;
    GPIO_State activeState;
    // private value
    ButtonState lastState;
    int32_t lastChangeTime;
    bool isEventAlreadySended;
} ButtonChannelDescriptor;

void initDeviceAsButton(DeviceManagerBase* base);

#endif // BUTTON_CHANNEL_DESCRIPTOR_H_

#ifndef LED_HARDWARE_DRIVER_H
#define LED_HARDWARE_DRIVER_H

#include <stdint.h>

typedef enum{LED_CHANNEL_1, LED_CHANNEL_2, LED_CHANNEL_3, LED_CHANNEL_4, LED_CHANNEL_5, LED_CHANNEL_6} LedChannel;

//set PWM signal on the specified channel
int setChannelPWM_Value(LedChannel channel, uint32_t value);

//prepare for PWM generating
int ledHardwareInit(LedChannel channel);
int ledHardwareDeinit(LedChannel channel);

#endif

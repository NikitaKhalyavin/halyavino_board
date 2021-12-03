#ifndef LED_PORT_H_
#define LED_PORT_H_

#include <stdint.h>

typedef uint32_t LedChannel;

//set PWM signal on the specified channel
int setChannelPWM_Value(LedChannel channel, uint32_t value);

//prepare for PWM generating
void ledHardwareInit(LedChannel channel);
void ledHardwareDeinit(LedChannel channel);

#endif // LED_PORT_H_

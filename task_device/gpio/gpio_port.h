#ifndef GPIO_PORT_H_
#define GPIO_PORT_H_

#include <stdint.h>

typedef uint32_t GPIO_Channel;
typedef enum {GPIO_STATE_HIGH, GPIO_STATE_LOW} GPIO_State;


void setGPIO_ChannelState(GPIO_Channel channel, GPIO_State state);

GPIO_State getGPIO_ChannelState(GPIO_Channel channel);

#endif // GPIO_PORT_H_

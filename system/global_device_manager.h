#ifndef GLOBAL_DEVICE_MANAGER_H
#define GLOBAL_DEVICE_MANAGER_H

#include "led_channel_manager.h"

void initAllDevices(void);

//for using from main only
void setLedFile(LedChannel channel, char* fileName);
void startAll(uint32_t startTime);

//for using from TIM interrupt only
void handleAllDevices(uint32_t currentTime);

#endif

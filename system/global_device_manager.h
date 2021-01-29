#ifndef GLOBAL_DEVICE_MANAGER_H
#define GLOBAL_DEVICE_MANAGER_H

#include "led_channel_manager.h"
#include "sound_manager.h"
#include "gpio_channel_manager.h"


void initAllDevices(void);

//for using from main only
void setLedFile(LedChannel channel, char* fileName);
void setWavFile(char* fileName);
void configButton(GPIO_Channel channel, char* fileName);

void startAll(uint32_t startTime);
void checkButtonsState(void);

//for using from TIM interrupt only
void handleAllDevices(uint32_t currentTime);
void handleSoundDevice(void);

#endif

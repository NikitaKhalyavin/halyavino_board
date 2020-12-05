#ifndef LED_CHANNEL_MANAGER_H
#define LED_CHANNEL_MANAGER_H

#include <led_hardware_driver.h>
#include <led_file.h>
#include <device_manager_status.h>


typedef void(*LinkLedFileToChannel)(void* this, const char* fileName);
typedef void(*SetHardwarePWM_Value)(void* this, float timeFromBeginning);
typedef void(*LedChannelStartRecording)(void* this);
typedef void(*LedChannelEndRecording)(void* this);


typedef struct
{
    DeviceStatus status;
    
    //use it only in main thread
    LedFileDescriptor * linkedFile;
    LinkLedFileToChannel linkFile;
    LedChannel channel;
    
    LedChannelStartRecording startRecording;
    
    //use it only in interrupt mode
    SetHardwarePWM_Value setValue;
    LedChannelEndRecording endRecording;
    
} LedChannelManager;

void ledChannelManagerInit(LedChannelManager* this);


#endif
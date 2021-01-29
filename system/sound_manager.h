#ifndef WAV_MANAGER_H
#define WAV_MANAGER_H

#include <wav_file.h>
#include <device_manager_status.h>


typedef void(*LinkWavFileToChannel)(void* this, const char* fileName);
typedef void(*SetHardwareSoundValue)(void* this);
typedef void(*WavChannelStartRecording)(void* this);
typedef void(*WavChannelEndRecording)(void* this);


typedef struct
{
    DeviceStatus status;
    
    //use it only in main thread
    WavFileDescriptor * linkedFile;
    LinkWavFileToChannel linkFile;
    
    WavChannelStartRecording startRecording;
    
    //use it only in interrupt mode
    SetHardwareSoundValue setValue;
    WavChannelEndRecording endRecording;
    
} WavChannelManager;

void wavChannelManagerInit(WavChannelManager* this);


#endif

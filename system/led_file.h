#ifndef LED_FILE_H
#define LED_FILE_H

#include "stdint.h"
#include "file_status.h"
#include "stream_file_buffer.h"

typedef struct
{
    float timeInSeconds;
    float value;
} LedFilePoint;


typedef void(*ReadFileFunction)(void * this, const char * fileName);
typedef uint32_t(*GetChannelValueFunction)(void * this, float currentTimeInSeconds);
typedef void(*CloseFileFunction)(void * this);
typedef void (*GoToNextPointFunction) (void* this);
    
typedef struct
{
    FileStatus status;
    int32_t pointNumber;
    int32_t currentPointIndex;
    
    LedFilePoint currentPoint;
    LedFilePoint nextPoint;
    
    ReadFileFunction readFile;
    GoToNextPointFunction goNext;
    GetChannelValueFunction getChannelValue;
    CloseFileFunction close;
    
    //private section
    //please, don't use this fields
    StreamFileBuffer buffer;
    
    
} LedFileDescriptor;

void ledFileDescriptorInit(LedFileDescriptor * descriptor);

#endif

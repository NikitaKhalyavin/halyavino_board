#ifndef LED_FILE_H
#define LED_FILE_H
#include "stdint.h"

typedef struct
{
    float timeInSeconds;
    float value;
} LedFilePoint;

typedef enum{LED_FILE_RECORDING, LED_FILE_ENDED, LED_FILE_ERROR, LED_FILE_EMPTY} FileStatus;

typedef void(*ReadFileFunction)(void * this, const char * fileName);
typedef uint32_t(*GetChannelValueFunction)(void * this, float currentTimeInSeconds);
typedef void(*CloseFileFunction)(void * this);
    
typedef struct
{
    FileStatus status;
    
    ReadFileFunction readFile;
    GetChannelValueFunction getChannelValue;
    CloseFileFunction close;
    
    //private section
    //please, don't use this fields
    uint8_t pointNumber;
    LedFilePoint * pointArray;
    
    
} LedFileDescriptor;

void ledFileDescriptorInit(LedFileDescriptor * descriptor);

#endif

#ifndef WAV_FILE_H
#define WAV_FILE_H

#include "stdint.h"
#include "file_status.h"
#include "stream_file_buffer.h"

typedef enum {PCM} FormatType;

typedef void(*ReadFileFunction)(void * this, const char * fileName);
typedef uint32_t(*GetChannelValueFunction)(void * this, float currentTimeInSeconds);
typedef void(*CloseFileFunction)(void * this);
    
typedef struct
{
    FormatType format;
    uint32_t channelNumber;
    uint32_t sampleRate;
    uint32_t bitsPerSample;
    uint32_t bytesPerFrame;
    
    FileStatus status;
    int32_t numberOfSamples;
    int32_t currentSampleIndex;
    
    ReadFileFunction readFile;
    GetChannelValueFunction getChannelValue;
    CloseFileFunction close;
    
    //private section
    //please, don't use this fields
    StreamFileBuffer buffer;
    uint8_t* tempBuffer;
    
    
} WavFileDescriptor;

void wavFileDescriptorInit(WavFileDescriptor * descriptor);

#endif

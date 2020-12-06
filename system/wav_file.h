#ifndef WAV_FILE_H
#define WAV_FILE_H

#include "stdint.h"
#include "file_status.h"


typedef void(*ReadFileFunction)(void * this, const char * fileName);
typedef uint8_t(*GetChannelValueFunction)(void * this);
typedef void(*CloseFileFunction)(void * this);

    
typedef struct 
{
    //to-do with the wav specification
    //for internal using only
    //it's okay to avoid unnecessary data saving
}WavFileHeader;    
    
typedef struct
{
    FileStatus status;
    
    //read file from spi and parse it.s header. 
    //file has to be written in the pair of short buffers, which change
    //each other during getting empty and load data asynchronous
    //header of file is also reading and checking here
    ReadFileFunction readFile;
    
    //get the next value from file. If it has more than 1 channel, values have to be mixed
    //output format must be controlled also
    GetChannelValueFunction getChannelValue;
    
    //delete paired buffers and other objects if necessary
    CloseFileFunction close;
    
    //private section
    //please, don't use this fields
    WavFileHeader header;
    PairedStreamBuf * buffer;
    
    
} LedFileDescriptor;

void ledFileDescriptorInit(LedFileDescriptor * descriptor);

#endif

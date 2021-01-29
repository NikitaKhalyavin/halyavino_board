#include "led_file.h"
#include "led_hardware_driver.h"

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static bool isLedFileHeaderCorrect(uint8_t* file);

static void readFileFunction (void * this, const char * fileName)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor*)this;
    //it will be rewrite if no errors occured
    descriptor->status = FILE_ERROR;
    
    const uint32_t LED_FILE_BUFFER_SIZE = 64;
    StreamBufferInitializingResult bufferRes = streamFileBufferInit(&(descriptor->buffer), 
                LED_FILE_BUFFER_SIZE, FILE_DEVICE_MEMORY, fileName);
    if(bufferRes != STREAM_BUFFER_INITIALIZING_OK)
        return;
    
    /*
    * led file header structure:
    * bytes 0-2: '0xAA', '0xBB', '0xCC'
    * byte 3: number of data points
    * other data: points in format:
    * 4 bytes - float time
    * 4 bytes - float value
    */
    
    //read the header and first point
    const uint32_t initializingReadingSize = 3 + 1 + 8; 
    uint8_t readBuff[initializingReadingSize];
    
    StreamBufferReadingResult res = descriptor->buffer.read((void*)(&(descriptor->buffer)), readBuff, initializingReadingSize);
    if(res != STREAM_BUFFER_READING_SUCCESS)
    {
        return;
    }
    
    if(!isLedFileHeaderCorrect(readBuff))
    {
        return;
    }

    descriptor->pointNumber = (uint8_t)(readBuff[3]);
    
    //defined by the file format
    const unsigned int indexOfDataStart = 4;
    
    const unsigned int numberOfBytesInFloat = 4;
    
    LedFilePoint point;
    point.timeInSeconds = *(float*)(&readBuff[indexOfDataStart]);
    point.value = *(float*)(&readBuff[indexOfDataStart + numberOfBytesInFloat]);
    descriptor->nextPoint = point;
    
    LedFilePoint dummyPoint;
    dummyPoint.timeInSeconds = NAN;
    dummyPoint.value = NAN;
    descriptor->currentPoint = dummyPoint;
    descriptor->currentPointIndex = -1;
    
    descriptor->status = FILE_RECORDING;
}

void goToNextPointFunction (void* this)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor*)this;
    
    const unsigned int numberOfBytesInFloat = 4;
    const unsigned int numberOfBytesInOnePointDescription = numberOfBytesInFloat*2;
    
    uint8_t readBuff[numberOfBytesInOnePointDescription];
    StreamBufferReadingResult res = descriptor->buffer.read((void*)(&(descriptor->buffer)), 
                            readBuff, numberOfBytesInOnePointDescription);
    if(res != STREAM_BUFFER_READING_SUCCESS)
    {
        descriptor->status = FILE_ERROR;
        return;
    }
    LedFilePoint point;
    point.timeInSeconds = *(float*)readBuff;
    point.value = *(float*)(&readBuff[numberOfBytesInFloat]);
    descriptor->currentPoint = descriptor->nextPoint;
    descriptor->nextPoint = point;
    descriptor->currentPointIndex++;
}

static uint32_t getChannelValueFunction(void * this, float currentTimeInSeconds)
{
    
    LedFileDescriptor * descriptor = (LedFileDescriptor*)this;
    
    if(descriptor->status != FILE_RECORDING)
        return 0;
    
    if(currentTimeInSeconds > descriptor->nextPoint.timeInSeconds)
    {
        if(descriptor->currentPointIndex < descriptor->pointNumber - 2)
            descriptor->goNext(this);
        else
        {
            descriptor->status = FILE_ENDED;
            return 0;
        }
    }
    
    //linear interpolating of the value between two points 
    float timeSinceLast = currentTimeInSeconds - descriptor->currentPoint.timeInSeconds;
    float fullTimeRangeLength = descriptor->nextPoint.timeInSeconds - descriptor->currentPoint.timeInSeconds;
    float startValue = descriptor->currentPoint.value;
    float endValue = descriptor->nextPoint.value;
    float deltaValue = endValue - startValue;
    float newValue = startValue + (deltaValue * timeSinceLast / fullTimeRangeLength);
    
    uint32_t resultValue = (uint32_t)(newValue * TIMER_COUNTER_MAX_VALUE);
    return resultValue;
}

static void closeFileFunction(void * this)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor *) this;
    deleteBuffer(&(descriptor->buffer));
    descriptor->status = FILE_EMPTY;
}
    
void ledFileDescriptorInit(LedFileDescriptor * descriptor)
{
    descriptor->readFile = readFileFunction;
    descriptor->getChannelValue = getChannelValueFunction;
    descriptor->close = closeFileFunction;
    descriptor->pointNumber = 0;
    descriptor->currentPointIndex = 0;
    descriptor->status = FILE_EMPTY;
    descriptor->goNext = goToNextPointFunction;
}


//return true if header of file is 0xAA, 0xBB, 0xCC
static bool isLedFileHeaderCorrect(uint8_t* file)
{
    if(file[0] != (uint8_t)0xAA)
        return false;
    if(file[1] != (uint8_t)0xBB)
        return false;
    if(file[2] != (uint8_t)0xCC)
        return false;
    return true;
    
}


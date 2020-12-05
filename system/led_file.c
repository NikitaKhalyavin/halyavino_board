#include "led_file.h"
#include "led_hardware_driver.h"
#include "small_file_reader.h"

#include <stdbool.h>
#include <stdlib.h>

static bool checkLedFileHeader(char* file);

void readFileFunction (void * this, const char * fileName)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor*)this;
    //it will be rewrite if no errors occured
    descriptor->status = LED_FILE_ERROR;
    
    uint8_t readBuff[512] = {170, 187, 204, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 63,
0, 0, 128, 63, 154, 153, 153, 63, 0, 0, 0, 0, 0, 0, 160, 64,
0, 0, 0, 63, 0, 0, 32, 65, 0, 0, 0, 0};
    unsigned int bytesRead = 44;
    
//    FileSystemResultMessage res = readSmallFileToBuffer(readBuff, &bytesRead, fileName);     
//    if(res != FILE_SUCCESS)
//    {
//        //to do: error handle
//        return;
//    }        
        
    descriptor->pointNumber = (uint8_t)(readBuff[3]);
    
    void* pointArray = malloc(descriptor->pointNumber * sizeof(LedFilePoint));
    descriptor->pointArray = (LedFilePoint*)pointArray;
    
    //defined by the file format
    const unsigned int indexOfDataStart = 4;
    
    for(int i = 0; i < descriptor->pointNumber; i++)
    {
        const unsigned int numberOfBytesInFloat = sizeof(float);
        const unsigned int numberOfBytesInOnePointDescription = numberOfBytesInFloat*2;
        uint8_t bytesOfTime[numberOfBytesInFloat];
        for(int byteNumber = 0; byteNumber < numberOfBytesInFloat; byteNumber++)
        {
            unsigned int cursor = indexOfDataStart + i*numberOfBytesInOnePointDescription + byteNumber;
            if (cursor >= bytesRead)
            {
                //error: value is not fully readed yet, but the file is ended already
                return;
            }
            bytesOfTime[byteNumber] = readBuff[cursor];
        }
        LedFilePoint point;
        point.timeInSeconds = *(float*)bytesOfTime;
        uint8_t bytesOfValue[numberOfBytesInFloat];
        for(int byteNumber = 0; byteNumber < numberOfBytesInFloat; byteNumber++)
        {
            unsigned int cursor = indexOfDataStart + i*numberOfBytesInOnePointDescription + numberOfBytesInFloat + byteNumber;
            if (cursor >= bytesRead)
            {
                //error: value is not fully readed yet, but the file is ended already
                return;
            }
            bytesOfValue[byteNumber] = readBuff[cursor];
        }
        point.value = *(float*)bytesOfValue;
        descriptor->pointArray[i] = point;
    }
    
    descriptor->status = LED_FILE_RECORDING;
}


static uint32_t getChannelValueFunction(void * this, float currentTimeInSeconds)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor*)this;
    if(currentTimeInSeconds <= descriptor->pointArray[0].timeInSeconds)
    {
        //this is start of file - value is zero
        return 0;
    }
    float maxTime = descriptor->pointArray[descriptor->pointNumber - 1].timeInSeconds;
    if(currentTimeInSeconds >= descriptor->pointArray[descriptor->pointNumber - 1].timeInSeconds)
    {
        //this is end of file - value is zero
        descriptor->status = LED_FILE_ENDED;
        return 0;
    }
    
    //if this moment is between start and end - calculate value between neightboor points
    LedFilePoint lastPoint;
    LedFilePoint nextPoint;
    for(int i = 1; i < descriptor->pointNumber; i++)
    {
        if(descriptor->pointArray[i].timeInSeconds >= currentTimeInSeconds)
        {
            nextPoint = descriptor->pointArray[i];
            lastPoint = descriptor->pointArray[i-1];
            break;
        }            
    }
    
    //linear interpolating of the value between two points 
    float timeSinceLast = currentTimeInSeconds - lastPoint.timeInSeconds;
    float fullTimeRangeLength = nextPoint.timeInSeconds - lastPoint.timeInSeconds;
    float startValue = lastPoint.value;
    float endValue = nextPoint.value;
    float deltaValue = endValue - startValue;
    float newValue = startValue + (deltaValue * timeSinceLast / fullTimeRangeLength);
    uint32_t resultValue = (uint32_t)(newValue*10000);
    
    return resultValue;
}

static void closeFileFunction(void * this)
{
    LedFileDescriptor * descriptor = (LedFileDescriptor *) this;
    void * dataArray = (void*) descriptor->pointArray;
    free(dataArray);
}
    
void ledFileDescriptorInit(LedFileDescriptor * descriptor)
{
    descriptor->readFile = readFileFunction;
    descriptor->getChannelValue = getChannelValueFunction;
    descriptor->close = closeFileFunction;
    descriptor->pointNumber = 0;
    descriptor->pointArray = NULL;
    descriptor->status = LED_FILE_EMPTY;
}


//return true if header of file is 0xAA, 0xBB, 0xCC
static bool checkLedFileHeader(char* file)
{
    if(file[0] != (char)0xAA)
        return false;
    if(file[1] != (char)0xBB)
        return false;
    if(file[2] != (char)0xCC)
        return false;
    return true;
    
}


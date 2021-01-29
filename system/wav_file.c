#include "wav_file.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

static bool isWavFileHeaderCorrect(uint8_t* file);

static void readFileFunction (void * this, const char * fileName)
{
    WavFileDescriptor * descriptor = (WavFileDescriptor*)this;
    //it will be rewrite if no errors occured
    descriptor->status = FILE_ERROR;
    
    const uint32_t WAV_FILE_BUFFER_SIZE = 512;
    StreamBufferInitializingResult bufferRes = streamFileBufferInit(&(descriptor->buffer), 
                WAV_FILE_BUFFER_SIZE, FILE_DEVICE_MEMORY, fileName);
    if(bufferRes != STREAM_BUFFER_INITIALIZING_OK)
        return;
    
    /*
    * wav file header structure:
    * bytes 0-3: "RIFF"
    * bytes 4-7: fileSize;
    * bytes 8-11: "WAVE"
    * bytes 12-15: "fmt\0"
    * bytes 16-19: 16 (formatDataLength)
    * bytes 20-21: formatType (1-PCM)
    * bytes 22-23: numberOfChannels 
    * bytes 24-27: SampleRate
    * bytes 28-31: ByteRate
    * bytes 32-33: BytesPerFrame
    * bytes 34-35: BitsPerSample
    * bytes 36-39: "data"
    * bytes 40-43: dataLength (bytes)
    */
    
    //read the header
    const uint32_t initializingReadingSize = 44; 
    uint8_t readBuff[initializingReadingSize];
    
    StreamBufferReadingResult res = descriptor->buffer.read((void*)(&(descriptor->buffer)), readBuff, initializingReadingSize);
    if(res != STREAM_BUFFER_READING_SUCCESS)
    {
        return;
    }
    
    if(!isWavFileHeaderCorrect(readBuff))
    {
        return;
    }

    descriptor->channelNumber = *((uint16_t*)(&readBuff[22]));
    descriptor->sampleRate = *((uint32_t*)(&readBuff[24])); // max value -  128000Hz
    descriptor->bitsPerSample = *((uint16_t*)(&readBuff[34]));
    descriptor->bytesPerFrame = *((uint16_t*)(&readBuff[32]));
    
    uint32_t dataFormat = *((uint16_t*)(&readBuff[20]));
    switch(dataFormat)
    {
        case 1:
            descriptor->format = PCM;
            break;
        default:
            return;
    }
    
    uint32_t dataLength = *((uint32_t*)(&readBuff[40]));
    descriptor->numberOfSamples = dataLength / descriptor->bytesPerFrame;
    descriptor->currentSampleIndex = 0;
    
    descriptor->tempBuffer = malloc(descriptor->bytesPerFrame * sizeof(uint8_t) );
    
    descriptor->status = FILE_RECORDING;
}



static uint32_t getChannelValueFunction(void * this, float currentTimeInSeconds)
{
    
    WavFileDescriptor * descriptor = (WavFileDescriptor*)this;
    
    if(descriptor->currentSampleIndex == descriptor->numberOfSamples)
        descriptor->status = FILE_ENDED;
    descriptor->currentSampleIndex++;
    
    if(descriptor->status != FILE_RECORDING)
        return 0;
    
    StreamBufferReadingResult res = descriptor->buffer.read((void*)(&(descriptor->buffer)), 
                descriptor->tempBuffer, descriptor->bytesPerFrame);
    if(res != STREAM_BUFFER_READING_SUCCESS)
    {
        descriptor->status = FILE_ERROR;
        return 0;
    }
    
    switch(descriptor->format)
    {
        case PCM:
        {
            uint32_t channelSum = 0;
            for(int index = 0; index < descriptor->channelNumber; index++)
            {
                uint32_t currentValue;
                switch(descriptor->bitsPerSample)
                {
                    case 8:
                        currentValue = (uint32_t)(descriptor->tempBuffer[index]);
                        break;
                    case 16:
                        currentValue = (uint32_t)(*(uint16_t*)(&(descriptor->tempBuffer[index * 2])));
                        break;
                    case 32:
                        currentValue = *(uint32_t*)(&(descriptor->tempBuffer[index * 4]));
                        break;
                    default:
                        descriptor->status = FILE_ERROR;
                        return 0;
                }
                channelSum += currentValue;
            }

            //get scale (256 - max value of ADC)
            uint32_t returnValue = channelSum * 256 / (descriptor->channelNumber * (1 << descriptor->bitsPerSample));
            if(returnValue > 255)
                return 0;
            return returnValue;
        }
        
        default:
            //error: unknown file format
            descriptor->status = FILE_ERROR;
            return 0;
    }
    /*
    const uint32_t arraySize = 64;
    const static int32_t sinus[arraySize] = {0, 3211, 6392, 9511, 12539, 15446, 18204, 20787, 23169, 25329, 27244, 28897, 30272, 31356, 32137, 32609, 32767, 32609, 32137, 31356, 30272, 28897, 27244, 25329, 23169, 20787, 18204, 15446, 12539, 9511, 6392, 3211, 0, -3211, -6392, -9511, -12539, -15446, -18204, -20787, -23169, -25329, -27244, -28897, -30272, -31356, -32137, -32609, -32767, -32609, -32137, -31356, -30272, -28897, -27244, -25329, -23169, -20787, -18204, -15446, -12539, -9511, -6392, -3211};
    WavFileDescriptor * descriptor = (WavFileDescriptor*)this;
    
    //array of values of sinus
    //this array was been generated automatically, there aren't any mistakes in it    
    static uint32_t index = 0;
    if(index >= arraySize)
    {
        
        index -= arraySize;
        //descriptor->status = FILE_ENDED;
        //return 0;
    }
    uint32_t returnValue = (sinus[index] >> 8) + 128;
    index += 4;
    return returnValue;*/
}

static void closeFileFunction(void * this)
{
    WavFileDescriptor * descriptor = (WavFileDescriptor *) this;
    deleteBuffer(&(descriptor->buffer));
    if(descriptor->tempBuffer != NULL)
    {
        free(descriptor->tempBuffer);
        descriptor->tempBuffer = NULL;
    }
    descriptor->status = FILE_EMPTY;
}
    
void wavFileDescriptorInit(WavFileDescriptor * descriptor)
{
    descriptor->readFile = readFileFunction;
    descriptor->getChannelValue = getChannelValueFunction;
    descriptor->close = closeFileFunction;
    descriptor->numberOfSamples = 0;
    descriptor->currentSampleIndex = 0;
    descriptor->tempBuffer = NULL;
    descriptor->status = FILE_EMPTY;
}


//return true if header of file is 0xAA, 0xBB, 0xCC
static bool isWavFileHeaderCorrect(uint8_t* file)
{
    /*
    * wav file header structure:
    * bytes 0-3: "RIFF"
    * bytes 4-7: fileSize;
    * bytes 8-11: "WAVE"
    * bytes 12-15: "fmt\0"
    * bytes 16-19: 16 (formatDataLength)
    * bytes 20-21: formatType (1-PCM)
    * bytes 22-23: numberOfChannels 
    * bytes 24-27: SampleRate
    * bytes 28-31: ByteRate
    * bytes 32-33: BytesPerFrame
    * bytes 34-35: BitsPerSample
    * bytes 36-39: "data"
    * bytes 40-43: dataLength (bytes)
    */
    
    
    //check bytes 0-3
    if(file[0] != (uint8_t)'R')
        return false;
    if(file[1] != (uint8_t)'I')
        return false;
    if(file[2] != (uint8_t)'F')
        return false;
    if(file[3] != (uint8_t)'F')
        return false;
    
    //check bytes 8-11
    if(file[8]  != (uint8_t)'W')
        return false;
    if(file[9]  != (uint8_t)'A')
        return false;
    if(file[10] != (uint8_t)'V')
        return false;
    if(file[11] != (uint8_t)'E')
        return false;
    
    //check bytes 12-15
    if(file[12] != (uint8_t)'f')
        return false;
    if(file[13] != (uint8_t)'m')
        return false;
    if(file[14] != (uint8_t)'t')
        return false;
    
    //check bytes 36-39
    if(file[36] != (uint8_t)'d')
        return false;
    if(file[37] != (uint8_t)'a')
        return false;
    if(file[38] != (uint8_t)'t')
        return false;
    if(file[39] != (uint8_t)'a')
        return false;
    
    if(*(uint32_t*)(&file[16]) != 16)
        return false;
    
    return true;
    
}


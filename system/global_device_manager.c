#include "global_device_manager.h"
#include <main.h>
#include <stdbool.h>

extern TIM_HandleTypeDef htim1;

const unsigned int numberOfLedChannels = 6;
static uint32_t timeOfStart = 0;

static LedChannelManager ledChannels[numberOfLedChannels];


void initAllDevices()
{
    for(int i = 0; i < numberOfLedChannels; i++)
    {
        ledChannelManagerInit(&(ledChannels[i]));
        LedChannel channel = (LedChannel)(i);
        ledChannels[i].channel = channel;
    }
}

void setLedFile(LedChannel channel, char* fileName)
{
    int channelIndex = (unsigned int)(channel);
    ledChannels[channelIndex].linkFile(&(ledChannels[channelIndex]),fileName);
}

void startAll(uint32_t startTime)
{
    for(int i = 0; i < numberOfLedChannels; i++)
    {
        ledChannels[i].startRecording(&(ledChannels[i]));
    }
    //activate TIM3 and it's interrupts for auto device controlling
    timeOfStart = startTime;
    //HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Base_Start_IT(&htim1);
  
}

void handleAllDevices(uint32_t currentTime)
{
    uint32_t timeSinceStart = currentTime - timeOfStart;
    float timeSinceStartInSeconds = (float)(timeSinceStart) / 1000;
    
    //this flag allows to exit recording mode when all files end 
    bool isAnyRecording = false;
    
    
    for(int i = 0; i < numberOfLedChannels; i++)
    {
        if(ledChannels[i].status == DEVICE_STATUS_RECORDING)
        {
            ledChannels[i].setValue(&(ledChannels[i]), timeSinceStartInSeconds);
            isAnyRecording = true;
        }
    }
    if(!isAnyRecording)
    {
        //recording is finished - deactivate TIM3
        //HAL_TIM_Base_Stop(&htim1);
        HAL_TIM_Base_Stop_IT(&htim1);
    }
    
}


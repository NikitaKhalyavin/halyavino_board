#include "global_device_manager.h"
#include <stdbool.h>
#include <calling_timer_control.h>
#include "script_parser.h"

const unsigned int NUMBER_OF_LED_CHANNELS = 6;
const unsigned int NUMBER_OF_GPIO_CHANNELS = 8;
static uint32_t timeOfStart = 0;

typedef enum {DEVICE_GLOBAL_STATUS_WAITING, DEVICE_GLOBAL_STATUS_RECORDING} DeviceGlobalStatus;

static LedChannelManager ledChannels[NUMBER_OF_LED_CHANNELS];
static GPIO_Manager gpios[NUMBER_OF_GPIO_CHANNELS];
static WavChannelManager soundManager;

static DeviceGlobalStatus status = DEVICE_GLOBAL_STATUS_WAITING;

void checkButtonsState()
{
    if( status == DEVICE_GLOBAL_STATUS_RECORDING)
        return;
    for(int i = 0; i < NUMBER_OF_GPIO_CHANNELS; i++)
    {
        GPIO_Channel channel = (GPIO_Channel)(i);
        if(gpios[i].mode == GPIO_MODE_BUTTON)
        {
            GPIO_ButtonState state = gpios[i].modeSpecified.button.getValue(&gpios[i]);
            if(state == GPIO_BUTTON_PRESSED)
            {
                executeScriptFile(gpios[i].modeSpecified.button.fileName);
                status = DEVICE_GLOBAL_STATUS_RECORDING;
                return;
            }
        }
    }
    
}

void configButton(GPIO_Channel channel, char* fileName)
{
    int channelIndex = (unsigned int)(channel);
    gpios[channelIndex].config(&gpios[channelIndex], GPIO_MODE_BUTTON);
    gpios[channelIndex].modeSpecified.button.setFile(&gpios[channelIndex], fileName);
}

void initAllDevices()
{
    for(int i = 0; i < NUMBER_OF_LED_CHANNELS; i++)
    {
        ledChannelManagerInit(&(ledChannels[i]));
        LedChannel channel = (LedChannel)(i);
        ledChannels[i].channel = channel;
    }
    for(int i = 0; i < NUMBER_OF_GPIO_CHANNELS; i++)
    {
        GPIO_Init(&(gpios[i]));
        GPIO_Channel channel = (GPIO_Channel)(i);
        gpios[i].channel = channel;
    }
    
    wavChannelManagerInit(&soundManager);
}

void setLedFile(LedChannel channel, char* fileName)
{
    int channelIndex = (unsigned int)(channel);
    ledChannels[channelIndex].linkFile(&(ledChannels[channelIndex]),fileName);
}

void setWavFile(char* fileName)
{
    soundManager.linkFile(&soundManager,fileName);
}

void startAll(uint32_t startTime)
{
    for(int i = 0; i < NUMBER_OF_LED_CHANNELS; i++)
    {
        ledChannels[i].startRecording(&(ledChannels[i]));
    }
    soundManager.startRecording(&soundManager);
    //allow handling of devices in timer's interrupts
    startCallingTimerWork();
    timeOfStart = startTime;
  
}

void handleAllDevices(uint32_t currentTime)
{
    uint32_t timeSinceStart = currentTime - timeOfStart;
    float timeSinceStartInSeconds = (float)(timeSinceStart) / 1000;
    
    //this flag allows to exit recording mode when all files end 
    bool isAnyRecording = false;
    
    
    for(int i = 0; i < NUMBER_OF_LED_CHANNELS; i++)
    {
        if(ledChannels[i].status == DEVICE_STATUS_RECORDING)
        {
            ledChannels[i].setValue(&(ledChannels[i]), timeSinceStartInSeconds);
            isAnyRecording = true;
        }
    }
    if(!isAnyRecording)
    {
        //deactivate timer's interrupts
        stopCallingTimerWork();
        if(soundManager.status == DEVICE_STATUS_FREE)
            status = DEVICE_GLOBAL_STATUS_WAITING;
        //return system to the wait state
    }
    
}

void handleSoundDevice()
{
    if(soundManager.status == DEVICE_STATUS_RECORDING)
    {
        soundManager.setValue(&soundManager);
    }
    else
    {
        
    }
}


#include "gpio_channel_manager.h"
#include <stdlib.h>
#include <string.h>

static GPIO_TypeDef* getGPIO_Port(GPIO_Channel channel)
{
    return GPIOA;
}

static uint16_t getGPIO_Pin(GPIO_Channel channel)
{
    switch(channel)
    {
        case GPIO_CHANNEL_1:
            return GPIO1_Pin;
        case GPIO_CHANNEL_2:
            return GPIO2_Pin;
        case GPIO_CHANNEL_3:
            return GPIO3_Pin;
        case GPIO_CHANNEL_4:
            return GPIO4_Pin;
        case GPIO_CHANNEL_5:
            return GPIO5_Pin;
        case GPIO_CHANNEL_6:
            return GPIO6_Pin;
        case GPIO_CHANNEL_7:
            return GPIO7_Pin;
        case GPIO_CHANNEL_8:
            return GPIO8_Pin;
    }
}

static void setFileName(void* this, char* fileName)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode != GPIO_MODE_BUTTON)
        return;
    if(manager->modeSpecified.button.fileName != NULL)
        free(manager->modeSpecified.button.fileName);
    uint32_t nameLength = strlen(fileName) + 1;
    manager->modeSpecified.button.fileName = malloc(nameLength * sizeof(char));
    memcpy(manager->modeSpecified.button.fileName, fileName, nameLength);
}


static bool getGPIO_ButtonValue(void* this)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode != GPIO_MODE_BUTTON)
        return false;
    GPIO_TypeDef* port = getGPIO_Port(manager->channel);
    uint16_t pin = getGPIO_Pin(manager->channel);
    
    if(manager->modeSpecified.button.isReadyForNextChange)
    {
        GPIO_PinState currentState = HAL_GPIO_ReadPin(port, pin);
        if ((currentState == GPIO_PIN_RESET) && (manager->modeSpecified.button.currentState == GPIO_BUTTON_RELAXED))
        {
            manager->modeSpecified.button.currentState = GPIO_BUTTON_PRESSED;
            manager->modeSpecified.button.isReadyForNextChange = false;
            manager->modeSpecified.button.lastChangeTime = HAL_GetTick();
        }
        if ((currentState == GPIO_PIN_SET) && (manager->modeSpecified.button.currentState == GPIO_BUTTON_PRESSED))
        {
            manager->modeSpecified.button.currentState = GPIO_BUTTON_RELAXED;
            manager->modeSpecified.button.isReadyForNextChange = false;
            manager->modeSpecified.button.lastChangeTime = HAL_GetTick();
        }
    }
    else
    {
        const uint32_t TIMEOUT = 300;
        if(HAL_GetTick() - manager->modeSpecified.button.lastChangeTime > TIMEOUT)
            manager->modeSpecified.button.isReadyForNextChange = true;
    }
    
    return (manager->modeSpecified.button.currentState == GPIO_BUTTON_PRESSED);
}

static void servoTimerOnCallback(void* this)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode != GPIO_MODE_SERVO)
        return;
    
    if(manager->modeSpecified.servo.currentState == GPIO_SERVO_ENABLED)
    {
        GPIO_TypeDef* port = getGPIO_Port(manager->channel);
        uint16_t pin = getGPIO_Pin(manager->channel);
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        
        //TO-DO: setup timer interrupt time
    }
}

static void servoTimerOffCallback(void* this)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode != GPIO_MODE_SERVO)
        return;
    
    GPIO_TypeDef* port = getGPIO_Port(manager->channel);
    uint16_t pin = getGPIO_Pin(manager->channel);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

static ServoResult setServoAngle(void* this, float angle)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode != GPIO_MODE_SERVO)
        return SERVO_INTERNAL_ERROR;
    
    const uint32_t minTimeMCS = 544;
    const uint32_t maxTimeMCS = 2400;
    const float minAngle = -90;
    const float maxAngle = 90;
    
    if((angle < minAngle) || (angle > maxAngle))
        return SERVO_UNREACHEABLE_ANGLE_ERROR;
    
    float relativePosition = (angle - minAngle) / (maxAngle - minAngle);
    uint32_t pulseTime = minTimeMCS + (maxTimeMCS - minTimeMCS) * relativePosition;
    manager->modeSpecified.servo.pulseDuration = pulseTime;
    
    return SERVO_SUCCESS;
}

static void configToMode(void* this, GPIO_Mode mode)
{
    GPIO_Manager* manager = (GPIO_Manager*)this;
    if(manager->mode == mode)
        return;
    switch(mode)
    {
        case GPIO_MODE_BUTTON:
        {
            GPIO_TypeDef* port = getGPIO_Port(manager->channel);
            uint16_t pin = getGPIO_Pin(manager->channel);
            
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            
            GPIO_InitTypeDef GPIO_InitStruct = {0};
            GPIO_InitStruct.Pin = pin;
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            HAL_GPIO_Init(port, &GPIO_InitStruct);

            manager->modeSpecified.button.getValue = getGPIO_ButtonValue;
            manager->mode = GPIO_MODE_BUTTON;
            manager->modeSpecified.button.isReadyForNextChange = true;
            manager->modeSpecified.button.getValue(this);
            manager->modeSpecified.button.setFile = setFileName;
            return;
        }
        case GPIO_MODE_SERVO:
        {
            GPIO_TypeDef* port = getGPIO_Port(manager->channel);
            uint16_t pin = getGPIO_Pin(manager->channel);
            
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            
            GPIO_InitTypeDef GPIO_InitStruct = {0};
            GPIO_InitStruct.Pin = pin;
            GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(port, &GPIO_InitStruct);

            manager->modeSpecified.servo.offCallback = servoTimerOffCallback;
            manager->modeSpecified.servo.onCallback = servoTimerOnCallback;
            manager->modeSpecified.servo.setAngle = setServoAngle;
            manager->mode = GPIO_MODE_SERVO;
            manager->modeSpecified.servo.setAngle(this, 0);
            manager->modeSpecified.servo.currentState = GPIO_SERVO_DISABLED;
            return;
        }
        default:
        {
            GPIO_TypeDef* port = getGPIO_Port(manager->channel);
            uint16_t pin = getGPIO_Pin(manager->channel);
            
            HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            
            GPIO_InitTypeDef GPIO_InitStruct = {0};
            GPIO_InitStruct.Pin = pin;
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(port, &GPIO_InitStruct);
            manager->mode = GPIO_MODE_OFF;
        }
    }
    
}

void GPIO_Init(GPIO_Manager* manager)
{
    manager->mode = GPIO_MODE_OFF;
    manager->config = configToMode;
}

#include "dac.h"
#include "main.h"

static GPIO_PinState getState(uint8_t maskedValue);

void setDacValue(uint8_t value)
{
    //set values of dac pins
    HAL_GPIO_WritePin(DAC_PIN0_GPIO_Port, DAC_PIN0_Pin, getState(value & (1 << 0) ) );
    HAL_GPIO_WritePin(DAC_PIN1_GPIO_Port, DAC_PIN1_Pin, getState(value & (1 << 1) ) );
    HAL_GPIO_WritePin(DAC_PIN2_GPIO_Port, DAC_PIN2_Pin, getState(value & (1 << 2) ) );
    HAL_GPIO_WritePin(DAC_PIN3_GPIO_Port, DAC_PIN3_Pin, getState(value & (1 << 3) ) );
    HAL_GPIO_WritePin(DAC_PIN4_GPIO_Port, DAC_PIN4_Pin, getState(value & (1 << 4) ) );
    HAL_GPIO_WritePin(DAC_PIN5_GPIO_Port, DAC_PIN5_Pin, getState(value & (1 << 5) ) );
    HAL_GPIO_WritePin(DAC_PIN6_GPIO_Port, DAC_PIN6_Pin, getState(value & (1 << 6) ) );
    HAL_GPIO_WritePin(DAC_PIN7_GPIO_Port, DAC_PIN7_Pin, getState(value & (1 << 7) ) );
}


void disableSpeaker()
{
    HAL_GPIO_WritePin(SPEAKER_SHUTDOWN_GPIO_Port, SPEAKER_SHUTDOWN_Pin, GPIO_PIN_RESET);
}

void enableSpeaker()
{
    HAL_GPIO_WritePin(SPEAKER_SHUTDOWN_GPIO_Port, SPEAKER_SHUTDOWN_Pin, GPIO_PIN_SET);    
}

static GPIO_PinState getState(uint8_t maskedValue)
{
    if(maskedValue)
        return GPIO_PIN_SET;
    return GPIO_PIN_RESET;
}

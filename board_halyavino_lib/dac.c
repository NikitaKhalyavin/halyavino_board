#include "dac.h"
#include "main.h"

static GPIO_PinState getState(uint8_t maskedValue);
extern TIM_HandleTypeDef htim2;


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
    HAL_TIM_Base_Stop_IT(&htim2);
    setDacValue(0);
}

void enableSpeaker()
{
    HAL_GPIO_WritePin(SPEAKER_SHUTDOWN_GPIO_Port, SPEAKER_SHUTDOWN_Pin, GPIO_PIN_SET);    
    HAL_TIM_Base_Start_IT(&htim2);
}


void setTimerSampleRate(uint32_t sampleRate)
{
    uint32_t maxFrequency = 48000000 / (htim2.Init.Prescaler + 1);
    uint32_t requiredPeriod = maxFrequency / sampleRate;
    
    htim2.Init.Period = requiredPeriod;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

static GPIO_PinState getState(uint8_t maskedValue)
{
    if(maskedValue)
        return GPIO_PIN_SET;
    return GPIO_PIN_RESET;
}

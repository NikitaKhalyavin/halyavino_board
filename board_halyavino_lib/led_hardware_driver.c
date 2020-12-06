#include "led_hardware_driver.h"

#include "main.h"
#include "status_led.h"
int setChannelPWM_Value (LedChannel channel, uint32_t value)
{
    switch(channel)
    {
        case LED_CHANNEL_1:
            TIM3->CCR3 = value;
            break;
        case LED_CHANNEL_2:
            TIM3->CCR4 = value;
            break;
        case LED_CHANNEL_3:
            TIM4->CCR4 = value;
            break;
        case LED_CHANNEL_4:
            TIM4->CCR3 = value;
            break;
        case LED_CHANNEL_5:
            TIM1->CCR2 = value;
            break;
        case LED_CHANNEL_6:
            TIM1->CCR1 = value;
            break;
        default:
            //error: unexisting led channel
            setStatusLedOn();
            return -1;
    }
    return 0;
}

void ledHardwareInit(LedChannel channel)
{
    extern TIM_HandleTypeDef htim1;
    extern TIM_HandleTypeDef htim3;
    extern TIM_HandleTypeDef htim4;
    
    switch(channel)
    {
        case LED_CHANNEL_1:
            TIM3->CCR3 = 0;
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
            break;
        case LED_CHANNEL_2:
            TIM3->CCR4 = 0;
            HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
            break;
        case LED_CHANNEL_3:
            TIM4->CCR4 = 0;
            HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
            break;
        case LED_CHANNEL_4:
            TIM4->CCR3 = 0;    
            HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);            
            break;
        case LED_CHANNEL_5:
            TIM1->CCR2 = 0;                 
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
            break;
        case LED_CHANNEL_6:
            TIM1->CCR1 = 0;
            HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
            break;
        default:
            //error: unexisting led channel
            setStatusLedOn();
    }   
}

void ledHardwareDeinit(LedChannel channel)
{
    extern TIM_HandleTypeDef htim1;
    extern TIM_HandleTypeDef htim3;
    extern TIM_HandleTypeDef htim4;
    
    switch(channel)
    {
        case LED_CHANNEL_1:
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3);
            HAL_GPIO_WritePin(LED_CHANNEL1_PIN_GPIO_Port, LED_CHANNEL1_PIN_Pin, GPIO_PIN_RESET);
            break;
        case LED_CHANNEL_2:
            HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
            HAL_GPIO_WritePin(LED_CHANNEL2_PIN_GPIO_Port, LED_CHANNEL2_PIN_Pin, GPIO_PIN_RESET);
            break;
        case LED_CHANNEL_3:
            HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_4);
            HAL_GPIO_WritePin(LED_CHANNEL3_PIN_GPIO_Port, LED_CHANNEL3_PIN_Pin, GPIO_PIN_RESET);
            break;
        case LED_CHANNEL_4:
            HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
            HAL_GPIO_WritePin(LED_CHANNEL4_PIN_GPIO_Port, LED_CHANNEL4_PIN_Pin, GPIO_PIN_RESET);
            break;
        case LED_CHANNEL_5:
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
            HAL_GPIO_WritePin(LED_CHANNEL5_PIN_GPIO_Port, LED_CHANNEL5_PIN_Pin, GPIO_PIN_RESET);
            break;
        case LED_CHANNEL_6:
            HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
            HAL_GPIO_WritePin(LED_CHANNEL6_PIN_GPIO_Port, LED_CHANNEL6_PIN_Pin, GPIO_PIN_RESET);
            break;
        default:
            //error: unexisting led channel
            setStatusLedOn();
    }
    
}

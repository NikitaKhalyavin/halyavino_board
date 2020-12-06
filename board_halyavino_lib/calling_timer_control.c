#include "calling_timer_control.h"
#include <main.h>

extern TIM_HandleTypeDef htim1;

void startCallingTimerWork(void)
{
    HAL_TIM_Base_Start_IT(&htim1);
}    

void stopCallingTimerWork(void)
{
    HAL_TIM_Base_Stop_IT(&htim1);
}

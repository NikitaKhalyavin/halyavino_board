#include "servo_port.h"
#include "servo_port_stm.h"

extern TIM_HandleTypeDef htim1;

//returns true if fails
bool servoSetTimer(uint32_t timeMCS)
{
#define MCS_TO_TICKS(x) (x * 4) 
		
		uint32_t timeInTicks = MCS_TO_TICKS(timeMCS);
		if(timeInTicks > TIMER_COUNTER_MAX_VALUE)
			return true;
		
		htim1.Instance->CCR3 = timeInTicks;
		
		__HAL_TIM_CLEAR_IT(&htim1, TIM_IT_CC3);
		htim1.Instance->DIER |= TIM_CHANNEL_3;
		
    return false;
}

void servoResetTimer(void)
{
		htim1.Instance->DIER &= ~TIM_CHANNEL_3;	
}


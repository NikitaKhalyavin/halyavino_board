
#include "status_led.h"
#include "main.h"

void setStatusLedOn()
{
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);
}

void setStatusLedOff()
{
    HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET);    
}

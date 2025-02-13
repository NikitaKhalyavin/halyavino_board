/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TIMER_COUNTER_MAX_VALUE 10000
#define DAC_PIN0_Pin GPIO_PIN_13
#define DAC_PIN0_GPIO_Port GPIOC
#define STATUS_LED_Pin GPIO_PIN_14
#define STATUS_LED_GPIO_Port GPIOC
#define SPEAKER_SHUTDOWN_Pin GPIO_PIN_15
#define SPEAKER_SHUTDOWN_GPIO_Port GPIOC
#define GPIO1_Pin GPIO_PIN_0
#define GPIO1_GPIO_Port GPIOA
#define GPIO2_Pin GPIO_PIN_1
#define GPIO2_GPIO_Port GPIOA
#define GPIO3_Pin GPIO_PIN_2
#define GPIO3_GPIO_Port GPIOA
#define GPIO4_Pin GPIO_PIN_3
#define GPIO4_GPIO_Port GPIOA
#define GPIO5_Pin GPIO_PIN_4
#define GPIO5_GPIO_Port GPIOA
#define GPIO6_Pin GPIO_PIN_5
#define GPIO6_GPIO_Port GPIOA
#define GPIO7_Pin GPIO_PIN_6
#define GPIO7_GPIO_Port GPIOA
#define GPIO8_Pin GPIO_PIN_7
#define GPIO8_GPIO_Port GPIOA
#define LED_CHANNEL1_PIN_Pin GPIO_PIN_0
#define LED_CHANNEL1_PIN_GPIO_Port GPIOB
#define LED_CHANNEL2_PIN_Pin GPIO_PIN_1
#define LED_CHANNEL2_PIN_GPIO_Port GPIOB
#define DAC_PIN2_Pin GPIO_PIN_2
#define DAC_PIN2_GPIO_Port GPIOB
#define SPI2_NSS_Pin GPIO_PIN_12
#define SPI2_NSS_GPIO_Port GPIOB
#define LED_CHANNEL6_PIN_Pin GPIO_PIN_8
#define LED_CHANNEL6_PIN_GPIO_Port GPIOA
#define LED_CHANNEL5_PIN_Pin GPIO_PIN_9
#define LED_CHANNEL5_PIN_GPIO_Port GPIOA
#define USB_DP_PULLUP_Pin GPIO_PIN_10
#define USB_DP_PULLUP_GPIO_Port GPIOA
#define DAC_PIN1_Pin GPIO_PIN_15
#define DAC_PIN1_GPIO_Port GPIOA
#define DAC_PIN3_Pin GPIO_PIN_3
#define DAC_PIN3_GPIO_Port GPIOB
#define DAC_PIN4_Pin GPIO_PIN_4
#define DAC_PIN4_GPIO_Port GPIOB
#define DAC_PIN5_Pin GPIO_PIN_5
#define DAC_PIN5_GPIO_Port GPIOB
#define DAC_PIN6_Pin GPIO_PIN_6
#define DAC_PIN6_GPIO_Port GPIOB
#define DAC_PIN7_Pin GPIO_PIN_7
#define DAC_PIN7_GPIO_Port GPIOB
#define LED_CHANNEL4_PIN_Pin GPIO_PIN_8
#define LED_CHANNEL4_PIN_GPIO_Port GPIOB
#define LED_CHANNEL3_PIN_Pin GPIO_PIN_9
#define LED_CHANNEL3_PIN_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

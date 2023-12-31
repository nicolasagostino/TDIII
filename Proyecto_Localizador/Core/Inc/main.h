/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
_Bool  Verificar_Boton(GPIO_TypeDef *BOTONx, uint16_t BOTON_Pin);
void Delay_ms(unsigned int);
void send_uart(char *string, uint8_t uart_a_enviar);
void Manejo_Led(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BOTON_PANICO_Pin GPIO_PIN_0
#define BOTON_PANICO_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

//#define DEBUG_ACELEROMETRO

#define EST_UP			0
#define EST_FALLING		1
#define EST_DOWN		2
#define EST_RISING		3
#define EST_VALIDO		4

#define	FALLING_TIME	2//(100ms)
#define	DOWN_TIME		2//(100ms)
#define	RISING_TIME		2//(100ms)

#define BOTON_PANICO BOTON_PANICO_GPIO_Port,BOTON_PANICO_Pin


#define UART_1	1
#define UART_2	2

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

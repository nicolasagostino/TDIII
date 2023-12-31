/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @author         : Nicolas Agostino
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "defines.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//TIMERS
_Bool flag_1s=0;
_Bool flag_50ms=0;
uint8_t contador_100ms;
uint8_t contador_20s;
//MPU6050

extern float Ax;
extern float Ay;
extern float Az;
extern float Gx;
extern float Gy;
extern float Gz;
uint8_t movimiento_brusco_on=0;
//Boton de Pánico
unsigned int delay_boton;
unsigned int estado_boton;

uint8_t cadena[3];
#ifdef USE_SIM808
//Modem
extern uint8_t mensaje_enviandose;
extern uint32_t contador_comando;
extern uint8_t comando_a_recibir;
extern uint8_t estado_envio_SMS;
extern uint8_t contador_seg;
#endif

#ifdef USE_GPS
//GPS
extern struct GPS_Data GPS_aux;
extern _Bool flag_primer_GPS;
#endif
//Led
uint8_t contador_led=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_OC_DelayElapsedCallback (TIM_HandleTypeDef *htim)
{
	uint32_t pulse;
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
		//Acá entra cada 1seg
		flag_1s=1;

		if(contador_seg!=0)
			contador_seg++;

		pulse = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (pulse + (2400)));
	}
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
		//Acá entra cada 50ms
		flag_50ms=1;
		pulse = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
		__HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (pulse + (120)));
	}
}


//********************************************************************************
// Función:				  Verificar_Boton
//
// Descripción:	Verifica que el boton se haya pulsado correctamente (anti-rebote)
//		 (Si se agregan más botones habría que hacer un buffer para estado_boton)
//********************************************************************************
_Bool Verificar_Boton(GPIO_TypeDef *BOTONx, uint16_t BOTON_Pin)
{
	switch(estado_boton)
	{
		  default:
		  case EST_UP:
			  if(HAL_GPIO_ReadPin(BOTONx, BOTON_Pin)==0)
			  {
				  estado_boton=EST_FALLING;
				  Delay_ms(FALLING_TIME);
			  }
			  break;

		  case EST_FALLING:
			  if(!delay_boton)
			  {
				if(HAL_GPIO_ReadPin(BOTONx, BOTON_Pin)==0)
				{
					estado_boton=EST_DOWN;
					Delay_ms(DOWN_TIME);
				}
				else
					estado_boton=EST_UP;
			  }
			  break;

		  case EST_DOWN:
			  if((!delay_boton)&&(HAL_GPIO_ReadPin(BOTONx, BOTON_Pin)==1))
			  {
				  estado_boton=EST_RISING;
				  Delay_ms(RISING_TIME);
			  }
			  break;

		  case EST_RISING:
			  if(!delay_boton)
			  {
				if(HAL_GPIO_ReadPin(BOTONx, BOTON_Pin)==1)
				{
					estado_boton=EST_UP;

					//Damos como valida la pulsación del botón
					if((BOTONx==BOTON_PANICO_GPIO_Port)&&(BOTON_Pin==BOTON_PANICO_Pin))
						return(true);
				}
				else
				{
					estado_boton=EST_DOWN;
					Delay_ms(DOWN_TIME);
				}
			  }
			  break;
	}

	return(false);
}

//********************************************************************************
// Función:				  Delay_ms
//
// Descripción:	Recargo el tiempo de delay para el pulsador
//				(Si se agregan más botones habría que hacer un buffer)
//********************************************************************************
void Delay_ms(unsigned int tiempo)
{
	delay_boton = tiempo;
}

//********************************************************************************
// Función:				  send_uart
//
// Descripción:	Envia una cadena de caracteres por la UART seleccionada
//********************************************************************************
void send_uart(char *string, uint8_t uart_a_enviar)
{
	uint8_t len = strlen(string);

	switch(uart_a_enviar)
	{
		case UART_1:
			HAL_UART_Transmit(&huart1, (uint8_t*) string, len, 2000);
			break;

		case UART_2:
			HAL_UART_Transmit(&huart2, (uint8_t*) string, len, 2000);
			break;

		default:
			break;
	}

}

//********************************************************************************
// Función:				  buffsize
//
// Descripción:	Calcula el tamaño del buffer
//********************************************************************************
int bufsize (char *buff)
{
	int i=0;
	while(*buff++ != '\0') i++;
	return i;
}
//********************************************************************************
// Función:				  Manejo_Led
//
// Descripción:	Se modifica la velocidad en la que titila el led segun el estado
//********************************************************************************
void Manejo_Led(void)
{

	//Aca entra cada 50ms;
	contador_led++;

#ifdef USE_SIM808
	if(mensaje_enviandose != 0)
	{
		//Si está enviando un SMS titila bien rápido (c/ 100ms)
		if(contador_led>1)
		{
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			contador_led=0;
		}
	}
	else if(GPS_aux.Estado != 'A')
	{
		//Si no tiene GPS titila rápido, pero no tanto como cuando manda un SMS (c/ 300ms)
		if(contador_led>5)
		{
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			contador_led=0;
		}
	}
	else
	{
#endif
		//En estado normal titila lento (c/ 1s)
		if(contador_led>19)
		{
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			contador_led=0;
		}

#ifdef USE_SIM808
	}
#endif

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	char BufferDebug[100];
	//uint8_t data_a[3];
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  mpu6050Init();
  //HAL_UART_Receive_IT(&huart1, &byte1, 1);

  HAL_UART_Receive_IT(&huart1, cadena, 1);
  HAL_UART_Receive_IT(&huart2, cadena, 1);

  send_uart("\n\r ****************************** \n\r",UART_1);
  send_uart("\n\r      Proyecto Localizador      \n\r",UART_1);
  send_uart("\n\r          Version 1.7           \n\r",UART_1);
  send_uart("\n\r ****************************** \n\r",UART_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */


#ifdef USE_SIM808
	//Verifico si tocó el botón de pánico
	if(Verificar_Boton(BOTON_PANICO))
	{
		Encolar_SMS(MSJ_BOTON_PANICO);
	}
#endif


	/*------------------- Divisor 50ms ---------------------*/
	if(flag_50ms)
	{
		flag_50ms=0;

		contador_100ms++;

		Manejo_Led();

#ifdef USE_SIM808
		if(mensaje_enviandose!=0)
		{
			contador_comando++;

			if(contador_comando > MAXIMA_ESPERA_COMANDOS)
			{
				if(estado_envio_SMS<6)
				{
					estado_envio_SMS=6;//Por si está tirando error pq ya está conectado
					contador_comando=0;
					comando_a_recibir = 0;
				}
				else
				{
					Cancelar_SMS();
				}

			}
		}
#endif

		//Boton de pánico
		if(delay_boton>0)
			delay_boton--;

		mpu6050Config();
		mpu6050GyroRead();
		mpu6050AccelRead();

		if((!movimiento_brusco_on)&&((Gx>VALOR_GIRO_BRUSCO)||(Gy>VALOR_GIRO_BRUSCO)||(Gz>VALOR_GIRO_BRUSCO)||(Gx<(-VALOR_GIRO_BRUSCO))||(Gy<(-VALOR_GIRO_BRUSCO))||(Gz<(-VALOR_GIRO_BRUSCO))))
		{
			movimiento_brusco_on=TIEMPO_MOV_BRUSCO;

			//Envío por puerto serie el valor de los ejes del Acelerómetro
			sprintf(BufferDebug,"Ax: %.2f | Ay: %.2f | Az: %.2f\n\r", Ax, Ay, Az);
			send_uart(BufferDebug,UART_1);
			sprintf(BufferDebug,"Gx: %.2f | Gy: %.2f | Gz: %.2f\n\r", Gx, Gy, Gz);
			send_uart(BufferDebug,UART_1);

#ifdef USE_SIM808
			Encolar_SMS(MSJ_MOV_BRUSCO);
#endif
		}

	}


	/*-------------------- Divisor 100ms --------------------*/
	if(contador_100ms>=2)
	{
		contador_100ms=0;

#ifdef USE_SIM808
		Enviar_SMS();
#endif
	}

	/*-------------------- Divisor 1seg --------------------*/


	if(flag_1s)
	{

#ifdef DEBUG_ACELEROMETRO
		//Envío por puerto serie el valor de los ejes del Acelerómetro
		sprintf(BufferDebug,"Ax: %.2f | Ay: %.2f | Az: %.2f\n\r", Ax, Ay, Az);
		send_uart(BufferDebug,UART_1);
		sprintf(BufferDebug,"Gx: %.2f | Gy: %.2f | Gz: %.2f\n\r", Gx, Gy, Gz);
		send_uart(BufferDebug,UART_1);
#endif

		if(movimiento_brusco_on > 0)
			movimiento_brusco_on--;

		flag_1s=0;

		contador_20s++;

		if(contador_20s > 19)
		{
			contador_20s=0;

			//Cada 20 seg guardo la ubicacion (Si tengo datos GPS y no estoy enviando un SMS)
			if(flag_primer_GPS==true && mensaje_enviandose==false)
			{
				send_uart("\n\r Guardo posicion en FS \n\r",UART_1);
				FS_Guardar_Ubicacion();
			}
		}
	}
	/*------------------------------------------------------*/
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 400000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 30000;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 2400;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 120;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_1);
  HAL_TIM_OC_Start_IT(&htim1, TIM_CHANNEL_2);
  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 4800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BOTON_PANICO_Pin */
  GPIO_InitStruct.Pin = BOTON_PANICO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BOTON_PANICO_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//********************************************************************************
// Función:				  HAL_UART_RxCpltCallback
//
// Descripción:	Recepción de datos por UART
//
//********************************************************************************
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if defined (USE_GPS) ||  defined (USE_SIM808)
	uint8_t dato_recibido = cadena[0];
#endif

	if(huart->Instance == USART1)
	{
		//Interrupción por recepción de datos por UART1 (GPS)

#ifdef USE_GPS
		Recepcion_GPS(dato_recibido);
#endif

		HAL_UART_Receive_IT(&huart1, cadena, 1);
	}
	else if(huart->Instance == USART2)
	{
		//Interrupción por recepción de datos por UART2 (Modem)

#ifdef USE_SIM808
		Recepcion_Modem(dato_recibido);
#endif

		HAL_UART_Receive_IT(&huart2, cadena, 1);
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  //Si entra acá es porque hubo algun error
	  //Verificar la frecuencia de los SPI, I2C, etc
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

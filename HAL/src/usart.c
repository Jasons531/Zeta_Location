/*
**************************************************************************************************************
*	@file	uart.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief usart串口1，print串口测试doma
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
 /**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);  ///不需要库原因：从速度考虑直接使用寄存器更好
	return 1;
}

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart5;
UART_HandleTypeDef hlpuart1;

UART_FIFO_Typedef_t usart_rs485;

UART_RX UART_RX_DATA1 = {0, {0}, 0, {0}, false, false};
UART_RX UART_RX_DATA2 = {0, {0}, 0, {0}, false, false};
UART_RX UART_RX_LPUART1 = {0, {0}, 0, {0}, false, false};

uint8_t rx_rs485_buff[100] = {0};
uint8_t tx_rs485_buff[100] = {0};

void InitUartFifo(void)
{
	FIFO_UartVarInit(&usart_rs485,&huart5,tx_rs485_buff,rx_rs485_buff,100,100,NULL,NULL,NULL);
	FIFO_UartEnableRxIT(&usart_rs485);
}

/* USART1 init function */

void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
	
	HAL_NVIC_SetPriority(USART1_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
	HAL_UART_Receive_IT(&huart1,UART_RX_DATA1.aRxBuffer, RXBUFFERSIZE);
}

/* LPUART1 init function */
void MX_LPUART1_UART_Init(void)
{
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 115200;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
	
	HAL_NVIC_SetPriority(AES_RNG_LPUART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(AES_RNG_LPUART1_IRQn);
	HAL_UART_Receive_IT(&hlpuart1,UART_RX_LPUART1.aRxBuffer, RXBUFFERSIZE);
}

/* USART2 init function */

void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;   ///GPS
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
	
	HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_UART_Receive_IT(&huart2,UART_RX_DATA2.aRxBuffer, RXBUFFERSIZE);
}
/* USART5 init function */


void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
	 if(uartHandle->Instance==LPUART1)
  {
  /* USER CODE BEGIN LPUART1_MspInit 0 */

  /* USER CODE END LPUART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();
  
    /**LPUART1 GPIO Configuration    
    PB10     ------> LPUART1_TX
    PB11     ------> LPUART1_RX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_LPUART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* LPUART1 interrupt Init */
  /* USER CODE BEGIN LPUART1_MspInit 1 */

  /* USER CODE END LPUART1_MspInit 1 */
  }
	
  else if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    GPIO_InitStruct.Pin = USART1_TX|USART1_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART1;
    HAL_GPIO_Init(USART1_IO, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = USART2_TX|USART2_RX;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
    HAL_GPIO_Init(USART2_IO, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();
  
    /**USART1 GPIO Configuration    
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX 
    */
    HAL_GPIO_DeInit(USART1_IO, USART1_TX|USART1_RX);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();
  
    /**USART2 GPIO Configuration    
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX 
    */
    HAL_GPIO_DeInit(USART2_IO, USART2_TX|USART2_RX);

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(USART2_IRQn);

  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if 1

	if(huart->Instance==LPUART1) ///Zeta
	{
		UART_RX_LPUART1.USART_RX_BUF[UART_RX_LPUART1.USART_RX_Len]=UART_RX_LPUART1.aRxBuffer[0];	
		DEBUG(3,"%02x ",UART_RX_LPUART1.USART_RX_BUF[UART_RX_LPUART1.USART_RX_Len]);	
		UART_RX_LPUART1.USART_RX_Len++;
		
		ZetaRecviceBuf.Uart_time = HAL_GetTick(  );
		UART_RX_LPUART1.Rx_State = true;
	
	 if(UART_RX_LPUART1.USART_RX_Len >= 516)
			UART_RX_LPUART1.USART_RX_Len = 0;
	}	
	
	else if(huart->Instance==USART1) ///UART1
	{
		if(UART_RX_DATA1.aRxBuffer[0] == 0x0d)  ///以'\r''\n'作为结束标记
		{
			UART_RX_DATA1.Rx_State = true;		
		}
		else if(UART_RX_DATA1.aRxBuffer[0] == 0x0a) ///以'\n'作为结束标记
		{
			if(UART_RX_DATA1.Rx_State)
			{			
				UART_RX_DATA1.Rx_State = false;
				DEBUG_APP(2,"%s",UART_RX_DATA1.USART_RX_BUF);	
				if( 0 == strcmp((char *)UART_RX_DATA1.USART_RX_BUF, "AT+EXITGPS"))
				{					
					SetGpsMode.LocationState = PATIONFAIL;
					
					Gps.Disable(  ); ///关闭GPS
					DEBUG_WARNING(2,"*** Now It Will EXit GPS Mode ***");
				}
				else if( 0 == strcmp((char *)UART_RX_DATA1.USART_RX_BUF, "AT+ZETATEST"))
				{
					////待定
					User.TestMode = true;
				}
				else if( 0 == strcmp((char *)UART_RX_DATA1.USART_RX_BUF, "AT+SLEEP"))
				{
					User.SleepTime = 1;
					DEBUG_WARNING(2,"*** Now The Sleep Time = 1 Min ***");
				}
				
				memset(UART_RX_DATA1.USART_RX_BUF, 0, UART_RX_DATA1.USART_RX_Len);			
			  UART_RX_DATA1.USART_RX_Len = 0;
			}
		}
		else
		{
			UART_RX_DATA1.USART_RX_BUF[UART_RX_DATA1.USART_RX_Len]=UART_RX_DATA1.aRxBuffer[0];	
			
			UART_RX_DATA1.USART_RX_Len++;
		}	
		
	 if(UART_RX_DATA1.USART_RX_Len >= 516)
			UART_RX_DATA1.USART_RX_Len = 0;
	}
	
	else if(huart->Instance==USART2) ///Gps
	{	
		if(UART_RX_DATA2.aRxBuffer[0] == 0x0d)  ///以'\r''\n'作为结束标记
		{
			UART_RX_DATA2.Rx_State = true;		
		}
		else if(UART_RX_DATA2.aRxBuffer[0] == 0x0a) ///以'\n'作为结束标记
		{
			if(UART_RX_DATA2.Rx_State)
			{			
				UART_RX_DATA2.Rx_State = false;
				
				int8_t GLL_State = 0;
				uint8_t PosfixCounter = 0;
				
			 if(GLL_State == strcmp((char *)UART_RX_DATA2.USART_RX_BUF, "$PMTK010,002*2D"))
					SetGpsMode.Start = true;
				else if(GLL_State == strcmp((char *)UART_RX_DATA2.USART_RX_BUF, "$PMTK001,314,3*36"))
				{
					SetGpsMode.Gpll = true;
					SetGpsMode.GpsOverTime = HAL_GetTick( );  ///记录GPS定位时间
				}

				if(UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len-6] == 'A')
				{
					PosfixCounter++;
					
					if(PosfixCounter>=20)
					{
							Gps.Disable(  );
						  SetGpsMode.LocationState = PATIONDONE;
					}
					
					memcpy(SetGpsMode.GLL, UART_RX_DATA2.USART_RX_BUF, UART_RX_DATA2.USART_RX_Len);
					SetGpsMode.GLL[UART_RX_DATA2.USART_RX_Len++]='\r';
          SetGpsMode.GLL[UART_RX_DATA2.USART_RX_Len++]='\n';
				}
				else if(UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len-6] == 'V')
				{
					DEBUG(3,"get poation false\r\n"); 
					memcpy(SetGpsMode.GLL, UART_RX_DATA2.USART_RX_BUF, UART_RX_DATA2.USART_RX_Len);
				}	
			}		
			 
			DEBUG(2,"%s\r\n",UART_RX_DATA2.USART_RX_BUF);
			memset(UART_RX_DATA2.USART_RX_BUF, 0, UART_RX_DATA2.USART_RX_Len);
			UART_RX_DATA2.USART_RX_Len = 0;
		}
		else
		{
			UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len]=UART_RX_DATA2.aRxBuffer[0];	
			DEBUG(3,"%c",UART_RX_DATA2.USART_RX_BUF[UART_RX_DATA2.USART_RX_Len]);	
			UART_RX_DATA2.USART_RX_Len++;
		}	
		
	 if(UART_RX_DATA2.USART_RX_Len >= 516)
			UART_RX_DATA2.USART_RX_Len = 0;
	}	
		
#endif	
}


/* USER CODE BEGIN 1 */


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
**************************************************************************************************************
*	@file		 ard.c
*	@author  Jason 
*	@version V0.1
*	@date    2018/07/09
*	@brief	 底层IO初始化
* @conn		 Jason_531@163.com
***************************************************************************************************************
*/
#include "board.h"

/*!
 * Flag to indicate if the MCU is Initialized
 */
bool McuInitialized = false;

void BoardInitClock( void )
{
	if( McuInitialized == false )
	{
		HAL_Init(  );
	
    /***************时钟初始化********************/
		SystemClockConfig(  );
        
		/* Enable Power Control clock */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE(); ///开启时钟
		
		RTC_Init(  );
		
		/*******************开启RTC中断*******************/
		HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);
		HAL_NVIC_EnableIRQ(RTC_IRQn);
									
		McuInitialized = true;	
	} 
	else
	{		
				/* Enable Power Control clock */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE(); ///开启时钟	
		SystemClockReConfig(  );		
	}
	
	/***************串口初始化********************/
	MX_USART1_UART_Init(  ); 
}

void BoardInitMcu( void )
{
	__disable_irq( );
	MMA845xInit(  );
	
	/** enable irq */
	__enable_irq( );
	
	TimerHwInit(  );
	
	UserCheckGps(  );
	/****************ADC初始化*******************/
	MX_ADC_Init(  );
					
	ZetaHandle.Init(  );

	ZetaHandle.PowerOn(  );
}

/*
 *	BoardDeInitMcu:	进入低功耗模式：停机，需要设置相应IO模式
 *	返回值: 				无
 */
void BoardDeInitMcu( void )
{ 
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Enable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
		
	/****************************************/
    /* Disable the Peripheral */	
	
	HAL_I2C_MspInit(&hi2c2);
	hi2c2.State = HAL_I2C_STATE_RESET;
	
	HAL_ADC_MspDeInit(&hadc);  ///OK 
	hadc.State = HAL_ADC_STATE_RESET;
	
	HAL_UART_DeInit(&hlpuart1);
	hlpuart1.gState = HAL_UART_STATE_RESET;
	
	///关闭UART1时钟
  HAL_UART_DeInit(&huart1);
	huart1.gState = HAL_UART_STATE_RESET;
	
	///关闭UART2时钟
	HAL_UART_DeInit(&huart2);
	huart2.gState = HAL_UART_STATE_RESET;
		
	HAL_TIM_Base_MspDeInit(&htim2);
	htim2.State = HAL_TIM_STATE_RESET;
	
	/*******************关闭SPI*********************/	
	GPIO_InitStructure.Pin = 0xFEFE;   ///GPIO_PIN_0	 GPIO_PIN_8  
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG; 
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
		
	GPIO_InitStructure.Pin = GPIO_PIN_All;   
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG; 
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
			
  GPIO_InitStructure.Pin = 0xFFFD;  /// PB1
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
              
	/*********************失能系统定时器************************/							
	SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk | ~SysTick_CTRL_ENABLE_Msk | ~SysTick_CTRL_TICKINT_Msk;
	
	/* Disable GPIOs clock */
	__HAL_RCC_GPIOC_CLK_DISABLE(	);
	__HAL_RCC_GPIOH_CLK_DISABLE(	);
    
}

/*
 *	BoardEnterStandby:	进入待机模式
 *	返回值: 						无
 */
void BoardEnterStandby(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_PWR_CLK_ENABLE();

	/* Enable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	
	GPIO_InitStructure.Pin = GPIO_PIN_All;   ///GPIO_PIN_All
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG; ///low_power,其它较高
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
	HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);

	HAL_ADC_MspDeInit(&hadc);  ///OK
	hadc.State = HAL_ADC_STATE_RESET;
	
	HAL_UART_DeInit(&hlpuart1);
	hlpuart1.gState = HAL_UART_STATE_RESET;
	
	///关闭UART1时钟
  HAL_UART_DeInit(&huart1);
	huart1.gState = HAL_UART_STATE_RESET;
	
	///关闭UART2时钟
	HAL_UART_DeInit(&huart2);
	huart2.gState = HAL_UART_STATE_RESET;
		
	HAL_TIM_Base_MspDeInit(&htim2);
	htim2.State = HAL_TIM_STATE_RESET;
	
	HAL_I2C_MspInit(&hi2c2);
	hi2c2.State = HAL_I2C_STATE_RESET;

	//关闭RTC相关中断，可能在RTC实验打开了
	__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RtcHandle,RTC_IT_WUT);
	__HAL_RTC_TIMESTAMP_DISABLE_IT(&RtcHandle,RTC_IT_TS);
	__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle,RTC_IT_ALRA|RTC_IT_ALRB);
	
	//清除RTC相关中断标志位
	__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
	__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RtcHandle,RTC_FLAG_TSF); 
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle,RTC_FLAG_WUTF);
	
	/* 禁用唤醒源:唤醒引脚PA0、PC13 */
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);

//	/* 清除所有唤醒标志位 */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  __HAL_RCC_BACKUPRESET_RELEASE();                    //备份区域复位结束
  __HAL_RTC_WRITEPROTECTION_ENABLE(&RtcHandle);     //使能RTC写保护

	/* 使能唤醒引脚：PA0做为系统唤醒输入 */
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	/* 进入待机模式 */
	HAL_PWR_EnterSTANDBYMode(		);
	
	HAL_SuspendTick();
	
	/* Disable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();

}

/*
 *	SystemClockReConfig:	休眠唤醒等待时钟恢复
 *	返回值: 				无
 */
void SystemClockReConfig( void )
{
    __HAL_RCC_PWR_CLK_ENABLE( );
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    /* Enable HSE */
    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );

    /* Wait till HSE is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* Enable PLL */
    __HAL_RCC_PLL_ENABLE( );

    /* Wait till PLL is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
    {
    }

    /* Select PLL as system clock source */
    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
    {
    }
		
		/*********************使能系统定时器************************/
		SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief 系统时钟初始化
  * @param 外部时钟16MHZ
  * @retval None
  */
void SystemClockConfig( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */	
  /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
	
	/**Configure the Systick interrupt time 1ms
  */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/*
**************************************************************************************************************
*	@file			main.c
*	@author 	Jason
*	@version  V0.6
*	@date    2018/07/18
*	@brief	 Zeta通讯
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timer.h"
#include "delay.h"
#include "board.h"
#include "user-app.h"

#define 	RTCTIKE	 

extern UART_HandleTypeDef 			UartHandle;
extern RTC_HandleTypeDef 				RtcHandle;



/*******************************************************************************************************************
  * @函数名称		main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无	
  *****************************************************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{			
	/**************时钟初始化************/
   BoardInitClock(  );
	
	 DEBUG(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__); 	 
   
	/**************定位器初始化************/
   LocationInit(  );

	/**************读取FLASH************/
   UserReadFlash(  );
	
	/**************电源开关初始化************/
	 UserKeyPinInit(  );
		
	/**************电源开关处理************/
	 UserKeyWakeupHandle(  );	
		
	 UserCheckGps(  );
											 	 						
	 UserCheckCmd(&UserZetaCheck[MAC]);

	 UserCheckCmd(&UserZetaCheck[COUNTER]);
	
	 UserCheckCmd(&UserZetaCheck[RSSI]);

	 UserSetHeart(0x00);

	 UserLocationVerion( QUERY_SEND_VER );
	 
	 ///通讯建立后再开启加速度传感器中断识别
	 HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	 
	 LocationInfor.HeartArrive = true;
	  
	 DEBUG_APP(2,"Battery = %d", CheckBattery(  ));
	 
	while (1)
   {			 
		UserLocatReport(  ); 
	} 
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG(2,"error\r\n");
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


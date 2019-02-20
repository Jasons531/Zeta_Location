/*
**************************************************************************************************************
*	@file			main.c
*	@author 	Jason
*	@version  V0.6
*	@date    2018/07/18
*	@brief	 ZetaͨѶ
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
  * @��������		main
  * @����˵��   ������ 
  * @�������   ��
  * @�������   ��
  * @���ز���   ��	
  *****************************************************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{			
	/**************ʱ�ӳ�ʼ��************/
   BoardInitClock(  );
	
	 DEBUG(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__); 	 
   
	/**************��λ����ʼ��************/
   LocationInit(  );

	/**************��ȡFLASH************/
   UserReadFlash(  );
	
	/**************��Դ���س�ʼ��************/
	 UserKeyPinInit(  );
		
	/**************��Դ���ش���************/
	 UserKeyWakeupHandle(  );	
		
	 UserCheckGps(  );
											 	 						
	 UserCheckCmd(&UserZetaCheck[MAC]);

	 UserCheckCmd(&UserZetaCheck[COUNTER]);
	
	 UserCheckCmd(&UserZetaCheck[RSSI]);

	 UserSetHeart(0x00);

	 UserLocationVerion( QUERY_SEND_VER );
	 
	 ///ͨѶ�������ٿ������ٶȴ������ж�ʶ��
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


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
											 	 						
	 UserCheckCmd(&UserZetaCheck[MAC]);

	 UserCheckCmd(&UserZetaCheck[COUNTER]);
	
	 UserCheckCmd(&UserZetaCheck[RSSI]);

	 UserSetHeart(0x00);
	 
	 LocationInfor.MotionState = InActive;
	 
	 LocatHandles->SetMode( WaitMode );
	 
	 UserLocationVerion( QUERY_SEND_VER );
	 
		while (1)
   {			 
			UserLocatReport(  ); 
		 	 	
#if 0	 
			char str[100] ="$GPGLL,2232.9085,N,11356.5973,E,111334.000,A,A*51";
			char GPLL[10];
			char N_Data[15] ;
			char N;
			char E_Data[15] ;

			double data_N = 0;
			double data_E = 0;
		 
			uint8_t len = 4;
		 
		 	ZetaSendBuf.Buf[0] = 0xff;
			ZetaSendBuf.Buf[1] = 0x00;
			
			ZetaSendBuf.Buf[3] = 0x02;

			sscanf(str, "%[^,]%*[,] %[^,]%*[,] %[^,]%*[,] %[^,]%*[,]", GPLL,N_Data, &N, E_Data);     ////取数到,截止，同时过滤,

			printf("The lowercase is: %s %s %c %s\r\n", GPLL,N_Data,N,E_Data);

			sscanf(N_Data, "%lf", &data_N);
			sscanf(E_Data, "%lf", &data_E);

			SetGpsMode.EastSpend = data_E * 10000;
			SetGpsMode.NorthSpend = data_N * 10000;

			printf("E_Data = %.4f N_Data = %.4f E = %d, N = %d\r\n",data_N,data_E, SetGpsMode.EastSpend,SetGpsMode.NorthSpend);

			SetGpsMode.South = false;
			SetGpsMode.West = false;

			printf("state = 0x%02x\r\n",0x30 | (SetGpsMode.West << 1) | (SetGpsMode.South << 0));

			ZetaSendBuf.Buf[len++] = 0x00 | (SetGpsMode.West << 1) | (SetGpsMode.South << 0);

			ZetaSendBuf.Buf[len++] = (SetGpsMode.EastSpend >> 20)&0xFF; ///28bit取高8bit，注意非32bit取运算
			ZetaSendBuf.Buf[len++] = (SetGpsMode.EastSpend >> 12)&0xFF;
			ZetaSendBuf.Buf[len++] = (SetGpsMode.EastSpend >> 4)&0xFF;

			ZetaSendBuf.Buf[len++] = (((SetGpsMode.EastSpend >> 0) & 0xF) << 4) | ((SetGpsMode.NorthSpend >> 24) & 0xF); ///28bit取高4bit

			ZetaSendBuf.Buf[len++] = (SetGpsMode.NorthSpend >> 16) & 0xFF;
			ZetaSendBuf.Buf[len++] = (SetGpsMode.NorthSpend >> 8) & 0xFF;
			ZetaSendBuf.Buf[len++] = (SetGpsMode.NorthSpend >> 0) & 0xFF;
			
			ZetaSendBuf.Buf[2] = len; /// +sensor_len
			ZetaSendBuf.Len = ZetaSendBuf.Buf[2];

			UserSend(&ZetaSendBuf);
			
			HAL_Delay(30000);

#endif

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


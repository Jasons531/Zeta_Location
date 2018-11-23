/*
**************************************************************************************************************
*	@file	gpio-board.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#include "board.h"
#include "gpio-board.h"

void EXTI0_1_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );

}

void EXTI2_3_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}


void EXTI4_15_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );

}

/**
  * 函数功能: 用于检测按键是否被长时间按下
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：1 ：按键被长时间按下  0 ：按键没有被长时间按下
  */
uint8_t CheckPowerkey(void)
{			
	uint8_t downCnt =0;																//记录按下的次数
	uint8_t upCnt =0;																//记录松开的次数			
 
	while(1)																		//死循环，由return结束
	{	
		/*************** 1S按键延时 ***************/
		HAL_Delay(10);																//延迟一段时间再检测
		if(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON)			//检测到按下按键
		{		
			downCnt++;																//记录按下次数
			upCnt=0;																//清除按键释放记录
			if(downCnt>=100)														//按下时间足够
			{
				DEBUG(2,"长按电源按钮 \r\n");
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///等待按键释放
				return 1; 															//检测到按键被时间长按下
			}
		}
		else 
		{
			upCnt++; 																//记录释放次数
			if(upCnt>5)																//连续检测到释放超过5次
			{
				DEBUG(2,"按下时间不足\r\n");		
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///等待按键释放
				return 0;															//按下时间太短，不是按键长按操作
			}
		}
	}
}

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	中断回调函数：处理中断事件----进行IO判断，处理相应的DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
				DEBUG(2,"关机 \r\n");
				BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"意外中断\r\n");
			}		

		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///防止中断误触发
			{
				DEBUG_APP(2,"PIN = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN));

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
			}
		
		break;
		
		case GPIO_PIN_8:
			
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8))
		{
			DEBUG_APP(2,);
			MMA8452MultipleRead(  );
		}

		break;
		
		case GPIO_PIN_13:	
		if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13))
		{
			DEBUG_APP(2,);
		}
		
		break;
		
		case GPIO_PIN_15:
			DEBUG_APP(2,);

		break;
		
		default	:
			break;
	}
}

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
		if(User.SleepWakeUp) ////休眠唤醒：或切换进待机模式---作用设备在RTC、PC13休眠模式，长按触发后可以进入待机关机模式，短按回到休眠状态
		{
			BoardInitClock(  );
			
			User.SleepWakeUp = false;
		}
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

uint32_t MMa8452qTime = 0;
uint32_t MotionStopTime = 0;

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	中断回调函数：处理中断事件----进行IO判断，处理相应的DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{	
	uint32_t SleepTime = 0;
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
					DEBUG(2,"关机 \r\n");
				
					/*************关闭加速度传感器************/
					MX_I2C2_Init(  );
					
					MMA845xStandby(  );
					BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"意外中断\r\n");
				
				////休眠唤醒：或切换进待机模式---作用设备在RTC、PC13休眠模式，长按触发后可以进入待机关机模式，短按：设置RTC闹钟时间，回到休眠状态
				if(PATIONNULL != LocatHandles->BreakState(  ))
				{
					SleepTime = GetCurrentSleepRtc(  );
				}
				else
					SleepTime = 10;
									
				DEBUG(2,"AlarmTime = %d \r\n", SleepTime);
								
				LocatHandles->SetMode( WaitMode );
				DEBUG_APP(2,);
				SetRtcAlarm(SleepTime); ///闹钟时间-当前时间
				UserIntoLowPower(  );
			}
		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///防止中断误触发
			{
				if(User.SleepWakeUp)
				{		
					BoardInitClock(  );
					
					BoardInitMcu(  );	
				}		
				DEBUG_APP(2,"PIN = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN));

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
				
				if(User.SleepWakeUp)
				{
					User.SleepWakeUp = false;				

					uint32_t SleepTime = GetCurrentSleepRtc(  );
																								
					DEBUG_APP(2,"AlarmTime = %d ", SleepTime);
					SetRtcAlarm(SleepTime); ///闹钟时间-当前时间
					UserIntoLowPower(  );
				}
			}
		
		break;
		
		case GPIO_PIN_8:
			
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8))
			{			
				if(User.SleepWakeUp)
				{		
					User.SleepWakeUp = false;									
					
					if(MotionMode != LocatHandles->GetMode(  )) ///移动模式下不触发重新定位
					{
						LocatHandles->SetMode( MotionStopMode );
						DEBUG_APP(2,"---- MotionStopMode!!! ---- ");
					}
					
					BoardInitClock(  );
					
					BoardInitMcu(  );				

					MMa8452qTime = HAL_GetTick(  );
					DEBUG_APP(2,"---- Motion Wake Up!!! ---- ");
				}		
				
				LocationInfor.MoveTimes = FlashRead16(MOVE_CONDITION_ADDR);
				
				LocationInfor.StopTimes = FlashRead16(MOVE_STOP_CONDITION_ADDR);
				
				MotionStopTime = HAL_GetTick(  ); ///记录停止运动时间
				
				if(PATIONNULL != LocatHandles->BreakState(  ))///非GPS定位才开启
				{
					MMA8452MultipleRead(  );	
											
					if((HAL_GetTick(  ) - MMa8452qTime) > LocationInfor.MoveTimes * 1000) ///防止定位过程，多次开启重新定位标志
					{
						if(MotionStopMode == LocatHandles->GetMode(  )) ///移动模式下不触发重新定位，同时RTC 移动周期唤醒
						{
							LocatHandles->SetMode( MotionMode );
							LocationInfor.BegainLocat = true;
							
							LocationInfor.MotionState = Active;
							DEBUG_APP(2,"---- BegainLocat -----");
							break;
						}
						else
						{
							LocationInfor.MotionState = MultActive; ///定位器一直触发状态，等待周期性上报状态
							DEBUG_APP(3,);
							break;
						}
					}
					DEBUG_APP(2,"MMa8452qTime %d", HAL_GetTick(  ) - MMa8452qTime);		
				}					
			}

		break;
		
		case GPIO_PIN_15:
			DEBUG_APP(2,);

		break;
		
		default	:
			break;
	}
}

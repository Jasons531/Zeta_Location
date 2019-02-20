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
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_3_IRQHandler( void )
{
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}

void EXTI4_15_IRQHandler( void )
{	
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
//			RtcHandle.State = HAL_RTC_STATE_RESET;
			BoardInitClock(  );
			
			LedInit(  );
			
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

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	中断回调函数：处理中断事件----进行IO判断，处理相应的DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{	
	uint32_t TimeOut = 0;
	
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
					DEBUG(2,"关机 \r\n");
					PowerOff(  );
				
					/*************关闭加速度传感器************/
					MX_I2C2_Init(  );				
					MMA845xStandby(  );
					BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"意外中断\r\n");
				
				////休眠唤醒：或切换进待机模式---作用设备在RTC、PC13休眠模式，长按触发后可以进入待机关机模式，短按：设置RTC闹钟时间，回到休眠状态									
				DEBUG_APP(2,"BreakState = %d %d",LocatHandles->BreakState(  ),User.SleepWakeUp);
				if(WaitMode == LocatHandles->GetMode(  )) ////移动与停止模式，不还原休眠
				{
					LocationInfor.MotionState = FailActive;
					DEBUG_APP(2,);
				}
			
			}
		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key：下行数据处理
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///防止中断误触发
			{
				if(User.SleepWakeUp)
				{		
					BoardInitClock(  );
					
					BoardInitMcu(  );	
				}		
				DEBUG_APP(2,"PIN = %d setheart = %d SleepWakeUp = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN), setheart, User.SleepWakeUp);

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
				
				if(setheart) ///当前设置心跳模式
				{
					DEBUG_APP(2," ******* Set AlarmTime ******** ");
					
					setheart = false;
										
					LocationInfor.HeartCycle = FlashRead16(HEART_CYCLE_ADDR);
				
					User.SleepTime = LocationInfor.HeartCycle * MINUTE;
				
					SetRtcAlarm(User.SleepTime); 
					HAL_TIM_Base_Stop_IT(&htim2);
					UserIntoLowPower(  );					
				}		
			  else
				{	
					if(User.SleepWakeUp && (QueryLocaMode != LocatHandles->GetMode(  )) ) ///休眠状态下，恢复休眠
					{
						User.SleepWakeUp = false;				

						int32_t SleepTime = GetCurrentSleepRtc(  );
																									
						DEBUG_APP(2,"AlarmTime = %d ", SleepTime);
						
						if(SleepTime!=0)
						{							
							ResetRtcAlarm(User.AlarmDate, SleepTime);
							HAL_TIM_Base_Stop_IT(&htim2);
							UserIntoLowPower(  );
						}
						else
						{
							if(User.AlarmDate > User.CurrentDate)
							{
								ResetRtcAlarm(User.AlarmDate, SleepTime);
								HAL_TIM_Base_Stop_IT(&htim2);
								UserIntoLowPower(  );
							}
						}
					}
				}
			}
		
		break;
		
		case MMA8452INT_1:
						
			if(HAL_GPIO_ReadPin(MMA8452INT_1_IO,MMA8452INT_1))
			{							
				if(User.SleepWakeUp) ////休眠恢复系统时钟
				{		
					BoardInitClock(  );
					
					BoardInitMcu(  );	
					
					GetCurrentSleepRtc(  );

					User.SleepWakeUp = false;	
					
					DEBUG_APP(2,"---- LocatHandles = %d ---- ",LocatHandles->GetMode(  ));
					
					HAL_TIM_Base_Start_IT(&htim2);
				}		
				 				
				if(!LocationInfor.MotionStart && !LocationInfor.SingleAlarm) ///心跳模式下，设置为移动模式，同时非一次报警模式
				{
					LocationInfor.MotionStart = true;
					LocationInfor.CollectMoveTime = HAL_GetTick(  );  ///记录运动时间
					
					DEBUG_APP(2,"---- Motion Wake Up!!! ---- ");  ///需要添加fail保护机制	stopmode 	MotionStart = false;																
				}				
				
				LocationInfor.MoveTimes = FlashRead16(MOVE_CONDITION_ADDR);
				
				LocationInfor.StopTimes = FlashRead16(MOVE_STOP_CONDITION_ADDR);
				
				LocationInfor.ProtecProcess = true;
								
				MMA8452MultipleRead(  );	
				
				LocationInfor.CollectMoveStopTime = HAL_GetTick(  ); ///多次刷新记录停止运动时间，防止时间停止时间出错
								
				TimeOut  = LocationInfor.MoveTimes;
				
				TimeOut *= 1000;
										
				if((HAL_GetTick(  ) - LocationInfor.CollectMoveTime) > TimeOut) ///防止定位过程，多次开启重新定位标志
				{
					if(InvalidActive == LocationInfor.MotionState) ///移动模式下不触发重新定位，同时RTC 移动周期唤醒
					{				
						if(WaitMode == LocatHandles->GetMode(  )) ///等待心跳执行完成
						{
							LocatHandles->SetMode( MotionMode );
						}		
						LocationInfor.MotionState = SingleActive;
													
						LocatHandles->SetState(PATIONNULL);	
											
						DEBUG_APP(2,"*** BegainLocat  TimeOut:%d ***",TimeOut/1000);
						break;
					}
				}
				DEBUG_APP(3,"MMa8452qTime %d WorkMode = %d", HAL_GetTick(  ) - LocationInfor.CollectMoveTime, LocatHandles->GetMode(  ));		
			}

		break;
		
		case GPIO_PIN_15:
			DEBUG_APP(2,);

		break;
		
		default	:
			break;
	}
}

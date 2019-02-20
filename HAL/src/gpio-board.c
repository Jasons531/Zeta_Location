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
  * ��������: ���ڼ�ⰴ���Ƿ񱻳�ʱ�䰴��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����1 ����������ʱ�䰴��  0 ������û�б���ʱ�䰴��
  */
uint8_t CheckPowerkey(void)
{			
	uint8_t downCnt =0;																//��¼���µĴ���
	uint8_t upCnt =0;																//��¼�ɿ��Ĵ���			
 
	while(1)																		//��ѭ������return����
	{	
		if(User.SleepWakeUp) ////���߻��ѣ����л�������ģʽ---�����豸��RTC��PC13����ģʽ��������������Խ�������ػ�ģʽ���̰��ص�����״̬
		{
//			RtcHandle.State = HAL_RTC_STATE_RESET;
			BoardInitClock(  );
			
			LedInit(  );
			
			User.SleepWakeUp = false;
		}
		/*************** 1S������ʱ ***************/
		HAL_Delay(10);																//�ӳ�һ��ʱ���ټ��
		if(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON)			//��⵽���°���
		{		
			downCnt++;																//��¼���´���
			upCnt=0;																//��������ͷż�¼
			if(downCnt>=100)														//����ʱ���㹻
			{
				DEBUG(2,"������Դ��ť \r\n");
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///�ȴ������ͷ�
				return 1; 															//��⵽������ʱ�䳤����
			}
		}
		else 
		{
			upCnt++; 																//��¼�ͷŴ���
			if(upCnt>5)																//������⵽�ͷų���5��
			{
				DEBUG(2,"����ʱ�䲻��\r\n");		
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///�ȴ������ͷ�
				return 0;															//����ʱ��̫�̣����ǰ�����������
			}
		}
	}
}

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	�жϻص������������ж��¼�----����IO�жϣ�������Ӧ��DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{	
	uint32_t TimeOut = 0;
	
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
					DEBUG(2,"�ػ� \r\n");
					PowerOff(  );
				
					/*************�رռ��ٶȴ�����************/
					MX_I2C2_Init(  );				
					MMA845xStandby(  );
					BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"�����ж�\r\n");
				
				////���߻��ѣ����л�������ģʽ---�����豸��RTC��PC13����ģʽ��������������Խ�������ػ�ģʽ���̰�������RTC����ʱ�䣬�ص�����״̬									
				DEBUG_APP(2,"BreakState = %d %d",LocatHandles->BreakState(  ),User.SleepWakeUp);
				if(WaitMode == LocatHandles->GetMode(  )) ////�ƶ���ֹͣģʽ������ԭ����
				{
					LocationInfor.MotionState = FailActive;
					DEBUG_APP(2,);
				}
			
			}
		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key���������ݴ���
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///��ֹ�ж��󴥷�
			{
				if(User.SleepWakeUp)
				{		
					BoardInitClock(  );
					
					BoardInitMcu(  );	
				}		
				DEBUG_APP(2,"PIN = %d setheart = %d SleepWakeUp = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN), setheart, User.SleepWakeUp);

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
				
				if(setheart) ///��ǰ��������ģʽ
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
					if(User.SleepWakeUp && (QueryLocaMode != LocatHandles->GetMode(  )) ) ///����״̬�£��ָ�����
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
				if(User.SleepWakeUp) ////���߻ָ�ϵͳʱ��
				{		
					BoardInitClock(  );
					
					BoardInitMcu(  );	
					
					GetCurrentSleepRtc(  );

					User.SleepWakeUp = false;	
					
					DEBUG_APP(2,"---- LocatHandles = %d ---- ",LocatHandles->GetMode(  ));
					
					HAL_TIM_Base_Start_IT(&htim2);
				}		
				 				
				if(!LocationInfor.MotionStart && !LocationInfor.SingleAlarm) ///����ģʽ�£�����Ϊ�ƶ�ģʽ��ͬʱ��һ�α���ģʽ
				{
					LocationInfor.MotionStart = true;
					LocationInfor.CollectMoveTime = HAL_GetTick(  );  ///��¼�˶�ʱ��
					
					DEBUG_APP(2,"---- Motion Wake Up!!! ---- ");  ///��Ҫ���fail��������	stopmode 	MotionStart = false;																
				}				
				
				LocationInfor.MoveTimes = FlashRead16(MOVE_CONDITION_ADDR);
				
				LocationInfor.StopTimes = FlashRead16(MOVE_STOP_CONDITION_ADDR);
				
				LocationInfor.ProtecProcess = true;
								
				MMA8452MultipleRead(  );	
				
				LocationInfor.CollectMoveStopTime = HAL_GetTick(  ); ///���ˢ�¼�¼ֹͣ�˶�ʱ�䣬��ֹʱ��ֹͣʱ�����
								
				TimeOut  = LocationInfor.MoveTimes;
				
				TimeOut *= 1000;
										
				if((HAL_GetTick(  ) - LocationInfor.CollectMoveTime) > TimeOut) ///��ֹ��λ���̣���ο������¶�λ��־
				{
					if(InvalidActive == LocationInfor.MotionState) ///�ƶ�ģʽ�²��������¶�λ��ͬʱRTC �ƶ����ڻ���
					{				
						if(WaitMode == LocatHandles->GetMode(  )) ///�ȴ�����ִ�����
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

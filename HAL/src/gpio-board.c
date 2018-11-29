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
		if(User.LowPower) ////���߻��ѣ����л�������ģʽ---�����豸��RTC��PC13����ģʽ��������������Խ�������ػ�ģʽ���̰��ص�����״̬
		{
			BoardInitClock(  );
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

uint32_t MMa8452qTime = 0;
uint32_t MotionStopTime = 0;


/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	�жϻص������������ж��¼�----����IO�жϣ�������Ӧ��DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{	
	uint32_t SleepTime = 0;
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
					DEBUG(2,"�ػ� \r\n");
				
					/*************�رռ��ٶȴ�����************/
					MX_I2C2_Init(  );
					
					MMA845xStandby(  );
					BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"�����ж�\r\n");
				if(Normal == User.LowPower) ////���߻��ѣ����л�������ģʽ---�����豸��RTC��PC13����ģʽ��������������Խ�������ػ�ģʽ���̰�������RTC����ʱ�䣬�ص�����״̬
				{
					if(PATIONNULL != LocatHandles->BreakState(  ))
					{
						SleepTime = GetCurrentSleepRtc(  );
					}
					else
						SleepTime = 10;
										
					DEBUG(2,"AlarmTime = %d \r\n", SleepTime);
					
					SetRtcAlarm(SleepTime); ///����ʱ��-��ǰʱ��
					UserIntoLowPower(  );
				}
			}		

		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///��ֹ�ж��󴥷�
			{
				DEBUG_APP(2,"PIN = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN));

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
			}
		
		break;
		
		case GPIO_PIN_8:
			
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8))
			{
				if(User.SleepWakeUp)
				{		
					User.SleepWakeUp = false;				
					BoardInitClock(  );
					
					DEBUG_APP(2,"GPIO_PIN_8 wkup low-power now");
					BoardInitMcu(  );				

					MMa8452qTime = HAL_GetTick(  );
										
					if(MotionMode != LocatHandles->GetMode(  ))
					{
						LocatHandles->SetMode( MotionMode );
					}

					DEBUG_APP(2,"MMa8452qTime111---- %d",MMa8452qTime);
				}			

				MMA8452MultipleRead(  );	
				
				LocationInfor.MoveTimes = FlashRead16(MOVE_CONDITION_ADDR);
				
				LocationInfor.StopTimes = FlashRead16(MOVE_STOP_CONDITION_ADDR);
				
				DEBUG_APP(2,"BreakState---- %d",LocatHandles->BreakState(  ));
				if(PATIONNULL != LocatHandles->BreakState(  ))
				{
					if((HAL_GetTick(  ) - MMa8452qTime) > LocationInfor.AlarmCycle * 1000) ///��ֹ��λ���̣���ο������¶�λ��־
					{
						DEBUG_APP(2,"---- start: %d-----",User.LowPower);
					
						if(Motion != User.LowPower) ///�ƶ�ģʽ�²��������¶�λ��ͬʱRTC �ƶ����ڻ���
						{
							LocationInfor.BegainLocat = true;
							
							DEBUG_APP(2,"---- BegainLocat -----");
						}
						else
						{
							LocationInfor.MotionState = Start;
						}
					}
					else
					{
						MotionStopTime = HAL_GetTick(  );
					}
				}
				DEBUG_APP(2,"MMa8452qTime %d", HAL_GetTick(  ) - MMa8452qTime);			
			}

		break;
		
		case GPIO_PIN_15:
			DEBUG_APP(2,);

		break;
		
		default	:
			break;
	}
}

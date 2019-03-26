/*
**************************************************************************************************************
*	@file	user-app.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	Ӧ�ò�
***************************************************************************************************************
*/
#include <math.h>
#include "user-app.h"
#include "Location.h"
#include "debug.h"
#include "Zeta.h"
#include "gps.h"
#include "led.h"

UserZeta_t UserZetaCheck[] = {
	{0x10, 1500, Payload}, ///��ѯmac
	{0x11, 1000, Payload}, ///��ѯ����ʱ��
	{0x12, 500,  Payload}, ///��ѯʣ����
	{0x13, 1000, Payload}, ///��ѯ��������
};

User_t User = {0, 1, 1, 0, 0, false, false};

static uint8_t DeviceInfo[4] = {0};

/*UserKeyPinInit���û��������ѳ�ʼ��
*������							 ��
*����ֵ��   				 ��
*/
void UserKeyPinInit(void)
{						
	GPIO_InitTypeDef GPIO_InitStruct;

	/* ʹ��(����)KEY���Ŷ�ӦIO�˿�ʱ�� */  
	__HAL_RCC_GPIOA_CLK_ENABLE(); 

	/* ����KEY2 GPIO:�ж�ģʽ���½��ش��� */
	GPIO_InitStruct.Pin = POWER_KEY;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // �ر�ע������Ҫʹ���ж�ģʽ,�����������ش���
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(POWER_KEY_IO, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 8, 0);	//�˴����ж����ȼ����ܱ�ϵͳʱ�ӵ����ȼ��ߣ����򰴼��������ò�����
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);			//�ж�ʹ��
}

/*UserKeyWakeupHandle��	�û����Ѵ���
*������									��
*����ֵ��   						��
*/
void UserKeyWakeupHandle(void)
{						
	///PA0���ѣ���ʼ������
	if(CheckPowerkey(		))
	{		
		/* ���������־λ */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RCC_CLEAR_RESET_FLAGS(  );
		
		///�����ʼ��
		BoardInitMcu(  );
		DEBUG(2,"���� \r\n");
		PowerOn(  );
	}
	else
	{
		DEBUG(2,"���� \r\n");	
		
		/********���ü��ٶȴ���������*********/
		MX_I2C2_Init(  );				
		MMA845xStandby(  );
		
		BoardEnterStandby(  );
	}		
}

/*UserCheckGps��	�û���ѯGPS��Ϣ
*������				��
*����ֵ��   		��
*/
void UserCheckGps(void)
{						
	if(PATIONNULL == LocatHandles->BreakState(  ) || PATIONINIT == LocatHandles->BreakState(  ))
	{
		Gps.Init(  );		
		DEBUG_APP(2,"*** start timer ***");	
		Gps.Set(  );
	}
}

/*UserLocationVerion	���û���λ������汾
*����									����
*����ֵ								����  
*/
void UserLocationVerion(uint8_t VerCmd)
{
	uint8_t Len 		 = 4;
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
	ZetaSendBuf.Buf[4] = VerCmd;
	
	ZetaSendBuf.Buf[5] = LocationInfor.Versions;
			
	ZetaSendBuf.Buf[2] = Len + 2;
	
	ZetaSendBuf.Len 	 = ZetaSendBuf.Buf[2];
	
	DEBUG_APP(2,"Versions: 0X%02X : ", LocationInfor.Versions);
	UserSend(&ZetaSendBuf);
			
	/********************�������*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

/*UserSendLocation	���û���ȡ��λ����Ϣ
*����								����
*����ֵ							����
*/
void UserSendLocation(uint8_t LocationCmd)
{
	uint8_t Len					= 4;
	
	memset(ZetaSendBuf.Buf, 0, 20);
	
	ZetaSendBuf.Buf[0] 		= 0xff;
	ZetaSendBuf.Buf[1] 		= 0x00;
	
	ZetaSendBuf.Buf[3] 		= 0x02;
	
	if(LocatHandles->BreakState(  ) == PATIONDONE)
	{
		memcpy1(&ZetaSendBuf.Buf[Len], LocatHandles->GetLoca( LocatHandles->Buf, LocationCmd ), 8);
		
		ZetaSendBuf.Buf[2] 	= Len + 8;
	}
	else if(LocatHandles->BreakState(  ) == PATIONFAIL)
	{
		LocationCmd = (LocationCmd)==0x00?(0x0f):++LocationCmd;
		
		ZetaSendBuf.Buf[4]	= LocationCmd << 2;
		
		ZetaSendBuf.Buf[5] 	= 0x00;
		ZetaSendBuf.Buf[6] 	= 0x00;
		ZetaSendBuf.Buf[7] 	= 0x00;
		ZetaSendBuf.Buf[8] 	= 0x00;
		
		ZetaSendBuf.Buf[9]	= 0x00;
		ZetaSendBuf.Buf[10] 	= 0x00;
		ZetaSendBuf.Buf[11] 	= 0x00;
				
		ZetaSendBuf.Buf[2] 	= Len + 8; ///5
	}
	
	ZetaSendBuf.Len 			= ZetaSendBuf.Buf[2];
	
	UserSend(&ZetaSendBuf);
	
	HAL_Delay(4000);
			
	/********************�������*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

int32_t CurrentSleepTime = 0;

/*UserLocatMotion	���û�����λ���ƶ���Ϣ
*����					����
*����ֵ				����
*/
void UserLocatMotion(void)
{
	uint16_t AlarmCycle = 0;
	
	if(LocationInfor.AlarmEnable && !LocationInfor.SingleAlarm)
	{		
		DEBUG_APP(3,"LocationInfor.SingleAlarm = %d MotionState = %d",LocationInfor.SingleAlarm, LocationInfor.MotionState);

		////���δ���
		if(SingleActive == LocationInfor.MotionState)
		{		
			DEBUG_APP(2,);							
			
			UserCheckGps(  );	
								
			while(PATIONNULL == LocatHandles->BreakState(  ));
			
			UserSendLocation(ALARM_REP_LOCA_SUCESS);
					
			LocationInfor.MotionHandle = true;	
			
			AlarmCycle = FlashRead32(ALARM_CYCLE_ADDR);
				
			if(0x55aa == AlarmCycle)
			{
				LocationInfor.AlarmCycle = 0;
			}
			else
			{
				LocationInfor.AlarmCycle = AlarmCycle;
			}		
			
			DEBUG_APP(2,"LocationInfor.AlarmCycle = %d",LocationInfor.AlarmCycle);
				
			///���������ٶȻ������������ڷ������������㣬���¼��ǰ����ʱ�䣺1.������ʱ�䵽������������ģʽ��������ɺ���������ʱ��
			///�������ߣ����µȴ���ֹͣ��Ž���WaitMode����ģʽ
			///2.������ʱ��û�����ȴ���ֹͣ��ֹͣ�����WaitMode(������ʱ����������ʱ��)������������Ҫ�����������߱�־λ
			if(0 == LocationInfor.AlarmCycle)
			{					
				///���²�����Ҫ����ڶ�ʱ���н���ʵʱ����
				LocationInfor.SingleAlarm = true;
			}	
			else
			{
				///���ٶ�һֱ������ͬʱ�����ϱ�������ѭ����ʱ��	
				LocationInfor.MotionState = MultActive; ///��λ��һֱ����״̬���ȴ��������ϱ�״̬
								
				User.TimerCounter 		  = 0;
			}
		}
	}
	else if(!LocationInfor.AlarmEnable)
	{		
		LocatHandles->SetMode( WaitMode );
		
		LocationInfor.MotionState = FailActive;
	}
}

/*UserLocatMotionStop	���û�����λ��ֹͣ��Ϣ
*����							����
*����ֵ						����
*/
void UserLocatMotionStop(void)
{	
	DEBUG_APP(2,"--- Mode --- %d MotionState = %d",LocatHandles->GetMode(  ),LocationInfor.MotionState);

	if(LocationInfor.MotionState == StopActive)///���ٶ�ֹͣ
	{	
		DEBUG_APP(2,"---- MotionStopMode ----");
		
		LocationInfor.SingleAlarm  = false;
		LocationInfor.MotionHandle = false;
		
		LocatHandles->SetState(PATIONNULL);	

		UserCheckGps(  );

		while(PATIONNULL == LocatHandles->BreakState(  ));

		UserSendLocation(MOVE_STATIC_LOCA_SUCESS); ///�Զ�ֹͣ������ֹͣ����ǰΪ�Զ�ֹͣģʽ
		LocatHandles->SetMode( WaitMode );
	}		
	else if(MultActive == LocationInfor.MotionState || SingleActive == LocationInfor.MotionState)
	{
		LocatHandles->SetMode( MotionMode );
		DEBUG_ERROR(2,"**** Protect MotionMode****");
	}
	else
	{
		LocatHandles->SetMode( WaitMode );
		DEBUG_ERROR(2,"**** Protect WaitMode****");
	}
}

/*UserLocatDetect			��������������
*����							����
*����ֵ						����
*/
void UserLocatDetect(void)
{
	uint32_t TimeOut = 0;
	
	///�ƶ�ģʽ���ƶ�����
	if(MotionMode == LocatHandles->GetMode(  )) 
	{
		if(LocationInfor.MotionHandle)
		{
			TimeOut = LocationInfor.StopTimes;
			
			TimeOut *= 1000;
			
			///�ƶ���ִ��������ֹͣ������
			if(HAL_GetTick(  ) - LocationInfor.CollectMoveStopTime > TimeOut)
			{
				LocatHandles->SetMode( MotionStopMode );

				LocationInfor.MotionState = StopActive;
				DEBUG_APP(2,"*** StopTimeOut:%d ***",TimeOut/1000);
				
				return;
			}
			
			///�ȴ��ƶ�ֹͣ�ж����������Ƿ񵽴
			if((0 == GetCurrentSleepRtc(  )) && (User.AlarmDate == User.CurrentDate))
			{		
				DEBUG_APP(2,"*** CollectMoveStopTime:%d ***",(HAL_GetTick(  ) - LocationInfor.CollectMoveStopTime)/1000);
				LocatHandles->SetMode( HeartMode );	
				
				LocatHandles->SetState( PATIONNULL );
				
				return;
			}
		}
	}	
	else if(WaitMode == LocatHandles->GetMode(  ) || HeartMode == LocatHandles->GetMode(  )) ///�쳣�������ƣ�������������δ���㴥������
	{
		if(LocationInfor.AlarmEnable)
		{			
			if(LocationInfor.ProtecProcess && LocationInfor.MotionStart)
			{			
				if(HAL_GetTick( ) - LocationInfor.CollectMoveStopTime > 2000)
				{
					LocationInfor.ProtecProcess = false;
					
					LocationInfor.MotionStart = false;
					
					DEBUG_APP(2,"*** LocationInfor.MotionStart:%d ***",LocationInfor.MotionStart);
					if(InvalidActive == LocationInfor.MotionState)
					{
						DEBUG_APP(2,"*** Protect TimeOut:%d ***",2);
						LocationInfor.MotionState = FailActive;
					}
				}
			}		
		}
		else
			LocationInfor.MotionState = FailActive;
	}	
	
	///�����ϱ����������ٶ�һֱ������ͬʱ��������û����
	if(LocationInfor.AlarmEnable)
	{
		if((MultActive == LocationInfor.MotionState) && User.TimerCounter > LocationInfor.AlarmCycle * MINUTE)
		{
			DEBUG_APP(2,);
			User.TimerCounter = 0;
			LocationInfor.MotionState = SingleActive;
														
			LocatHandles->SetState(PATIONNULL);
		}
	}
	else ///�˶����̽��յ��رձ����������л���ֹͣģʽ
	{
		if(LocationInfor.MotionHandle)
		{
			LocatHandles->SetMode( MotionStopMode );
			
			LocationInfor.MotionState = StopActive;
		}
	}
	
	User.TimerCounter ++;
}

/*UserLocatReport	���û���λ����Ϣ�ϱ�
*����							����
*����ֵ						����
*/
void UserLocatReport(void)
{
	switch(LocatHandles->GetMode(  ))
	{
		case WaitMode:
			
			if(LocationInfor.HeartArrive)
			{
				LocatHandles->SetMode( HeartMode );
				
				break;
			}
				
			if(LocationInfor.MotionState == StopActive || LocationInfor.MotionState == FailActive) ///���������������ڣ����ٴ�����
			{		
				LocationInfor.MotionStart = false;
				LocationInfor.MotionState = InvalidActive;
				
				CurrentSleepTime = GetCurrentSleepRtc(  );
							
				DEBUG_APP(2,"*** LocationInfor.MotionStart:%d ***",LocationInfor.MotionStart);
				DEBUG_APP(2,"---- Sleep Again ----");					
				
				if(CurrentSleepTime > 0)
				{
					ResetRtcAlarm(User.AlarmDate, CurrentSleepTime);					
					HAL_TIM_Base_Stop_IT(&htim2);
					UserIntoLowPower(  );
				}
				else
				{
					if(User.AlarmDate > User.CurrentDate)
					{
						ResetRtcAlarm(User.AlarmDate, CurrentSleepTime);						
						HAL_TIM_Base_Stop_IT(&htim2);						
						UserIntoLowPower(  );
					}
					else if(User.AlarmDate == User.CurrentDate)//����ʱ�䵽��
					{					
						LocatHandles->SetState(PATIONNULL);
						LocatHandles->SetMode( HeartMode );	
						break;
					}
				}
			}		
			
			break;
		
		case	HeartMode:
					
			UserCheckGps(  );
		
			DEBUG_APP(2,"---- HeartMode ----");
						
			LocationInfor.HeartArrive = false;
					
			while(PATIONNULL == LocatHandles->BreakState(  ));
	
			DEBUG_APP(2,"LocationInfor.MotionState = %d GetMode = %d",LocationInfor.MotionState,LocatHandles->GetMode(  ));

			UserSendLocation(HEART_REPORT_SUCESS);
	
			LocationInfor.HeartCycle = FlashRead16(HEART_CYCLE_ADDR);
	
			DEBUG_APP(2,"LocationInfor.HeartCycle = %d",LocationInfor.HeartCycle);
	
			User.SleepTime = LocationInfor.HeartCycle * MINUTE;
				
		  ////�ƶ�ģʽ�ȴ��ƶ�ֹͣ���л����ƶ�ģʽ
			if(MultActive == LocationInfor.MotionState || SingleActive == LocationInfor.MotionState)
			{
				SetRtcAlarm(User.SleepTime);  
				
				LocatHandles->SetMode( MotionMode );	
								
				DEBUG_APP(2, " ***** MotionMode ****** ");										
				break;
			}
			
			DEBUG_APP(2,"---- LocatHandles = %d ---- ",LocatHandles->GetMode(  ));
			
			HAL_TIM_Base_Stop_IT(&htim2);
			
			LocationInfor.MotionStart = false;
			
			DEBUG_APP(2,"*** LocationInfor.MotionStart:%d ***",LocationInfor.MotionStart);
			
			LocatHandles->SetMode( WaitMode );
			LocationInfor.MotionState = InvalidActive;
			
			SetRtcAlarm(User.SleepTime);  
			
			DEBUG_APP(2,"---- Sleep ---- ");			
			UserIntoLowPower(  );

			break;
		
		case	MotionMode:
			
			UserLocatMotion(  );
				
			break;
		
		case	MotionStopMode:
			
			UserLocatMotionStop(  );
	
			break;
		
		case	QueryLocaMode:
			
			LocatHandles->SetState(PATIONNULL);	
			
			UserCheckGps(  );
	
			while(PATIONNULL == LocatHandles->BreakState(  ));
	
			UserSendLocation(QUERY_FEED_LOCA_SUCESS);
		
			HAL_TIM_Base_Stop_IT(&htim2);
		
			DEBUG_APP(2, "LocationInfor.MotionState = %d\r\n",LocationInfor.MotionState);
			
			if(SingleActive != LocationInfor.MotionState || MultActive != LocationInfor.MotionState) ///������
			{
				LocationInfor.MotionState = StopActive;
				LocatHandles->SetMode( WaitMode );
			}	
					
			break;
		
		default :
			break;
	}
}

/*UserSend���û�����Zeta���ͺ�����ע�⣺��������ǰ����ȴ�ģ��ע����ɣ�������ʧ�ܣ�����ģʽĬ�Ͽ�ֱ��ִ��,
*						���������MAX = 49
*������			��
*����ֵ��   ��
*/
void UserSend(Zeta_t *SendBuf)
{
	uint8_t ApplyCounter = 0;
	uint8_t i = 0;
	
	for(; i < 10; i++)
	{	
		DEBUG_APP(2,"start send data counter: %d\r\n", i);
		
		for(uint8_t j = 0; j<SendBuf->Len; j++)
		DEBUG(2,"%02X ",SendBuf->Buf[j]);
		DEBUG(2,"\r\n");
		
		ZetaHandle.Send(SendBuf);
		
		HAL_Delay(200);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((DataAck != Status) && (HAL_GetTick(  ) - overtime < 250));
					
		if(DataAck == Status)
		{			
//			HAL_Delay(300);	
//			UserCheckCmd(&UserZetaCheck[NETIME]); ///����ⲿflashʹ��
			return;
		}
		else if(LenError != Status)
		{			
			if(Unregistered == Status)
			{
				ApplyCounter ++;
				DEBUG(2,"---Writing registered---\r\n");
			
				HAL_Delay(6000);				
			}
			else
			{
				DEBUG_APP(2,"Status = %d\r\n",Status);
				HAL_Delay(1000);	
			}
		}
		else
		{		
			return;		
		}			
	}
}

/*UserSendTest������Zeta��������
*������			    ��
*����ֵ��       ��
*/
void UserSendTest(void)
{
	uint8_t len= 3;
	volatile uint16_t	UpSeqCounter = 1; 
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
		
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
	ZetaSendBuf.Buf[4] = (0x01 << 4); ///|���״̬
	
	/********************�豸ID*****************/
	memcpy(&ZetaSendBuf.Buf[5], &DeviceInfo[0], 4); 
	
	ZetaSendBuf.Buf[9] = 0x11; //֡��
			
	uint8_t data[3] = {1,2,3};			
	memcpy(&ZetaSendBuf.Buf[10], &data[0], len); 
	
	ZetaSendBuf.Buf[4] |= User.BatState;
	
	DEBUG(2,"ZetaSendBuf: ");
	for(uint8_t i = 0; i < len; i++)
	DEBUG(2,"%02X ",ZetaSendBuf.Buf[10+i]);
	DEBUG(2,"\r\n");
	
	ZetaSendBuf.Buf[10 + len++] = (UpSeqCounter&0xff00)<<8; ///Seq
	ZetaSendBuf.Buf[10 + len++] = (UpSeqCounter&0xff);
	
	ZetaSendBuf.Buf[10 + len] = ZetaHandle.CRC8(&ZetaSendBuf.Buf[10],len); ///CRC

	len ++;
		
	ZetaSendBuf.Buf[2] = 0x0A + len; /// +sensor_len
	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
	UserSend(&ZetaSendBuf);
	
	UpSeqCounter ++;
		
	/********************�������*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}


bool setheart = false;
/*UserDownCommand���������ݴ���
*������			    ��
*����ֵ��      	 ��
*/
void UserDownCommand(void)
{
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
		
	memcpy1(&ZetaSendBuf.Buf[4], LocatHandles->Cmd(&ZetaRecviceBuf), \
	ZetaSendBuf.Len);

	ZetaSendBuf.Buf[2] = 0x04 + ZetaSendBuf.Len; 

	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
	DEBUG_APP(2,"len = %d GetMode = %d\r\n",ZetaSendBuf.Len, LocatHandles->GetMode(  ));
	
	if(QUERY_DEV_LOCA != ZetaSendBuf.Buf[4] && ZetaRecviceBuf.Ack)
	{
		UserSend(&ZetaSendBuf);
		
		HAL_Delay(4000);
	}
	
	if(!ZetaRecviceBuf.Ack)
	{
		ZetaRecviceBuf.Ack = true;
	}
	
	if(ZetaSendBuf.Buf[4] == HEART_SET_CYCLE)
		setheart = true;
	memset(ZetaSendBuf.RevBuf, 0, ZetaSendBuf.Len - 0x04);
}

/*UserCheckCmd���û���ѯZeta����������ѯ�·�
*������					UserZetaCheckCmd����ѯ����
*����ֵ��   		��
*/
void UserCheckCmd(UserZeta_t *UserZetaCheckCmd)
{	
	uint8_t temp[3] = {0xff, 0x00, 0x04};
		
	memcpy(&ZetaSendBuf.Buf[0],&temp[0],3);
	
	ZetaSendBuf.Buf[3] = UserZetaCheckCmd->Cmd;
	
	ZetaSendBuf.Len = 4;
	
	for(uint8_t i = 0; i < 3; i++)
	{	
		for(uint8_t j = 0; j<4; j++)
		DEBUG(2,"%02x ",ZetaSendBuf.Buf[j]);
		DEBUG(2,"\r\n");
		
		ZetaHandle.Send(&ZetaSendBuf);
				
		HAL_Delay(100);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((UserZetaCheckCmd->Expect_retval != Status) && (HAL_GetTick(  ) - overtime < UserZetaCheckCmd->Timeout)); 
		
		if(UserZetaCheckCmd->Expect_retval == Status)
			break;
		else
		{			
			if(Unregistered == Status)
			{
				DEBUG(2,"---Writing registered---\r\n");
				HAL_Delay(6000);
			}
			else
			{
				HAL_Delay(300);
			}
		}
	}
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	memset(ZetaRecviceBuf.Buf, 0, ZetaRecviceBuf.Len);
}

/*UserSetHeart���û�����Zeta����
*������					mode: 0--����ģʽ6Hһ��������1--����ģʽ10Sһ������
*����ֵ��   		��
*/
void UserSetHeart(uint8_t mode)
{	
	uint8_t temp[4] = {0xff, 0x00, 0x05, 0x22};
		
	memcpy(&ZetaSendBuf.Buf[0],&temp[0],4);
	
	ZetaSendBuf.Buf[4] = mode;
	
	ZetaSendBuf.Len = 5;
	
	for(uint8_t i = 0; i < 3; i++)
	{		
		for(uint8_t j = 0; j<5; j++)
		DEBUG(2,"%02x ",ZetaSendBuf.Buf[j]);
		DEBUG(2,"\r\n");
		
		ZetaHandle.Send(&ZetaSendBuf);
		
		HAL_Delay(100);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((SetSecess != Status) && (HAL_GetTick(  ) - overtime < 200)); 
		
		if(SetSecess == Status)
			break;
		else
		{
			HAL_Delay(300);
		}
	}
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	memset(ZetaRecviceBuf.Buf, 0, ZetaRecviceBuf.Len);
}

/*UserSetTimer������Zeta��ʱ��ʱ��
*������					Timer�������жϿ�ʼʱ�䡢���ʱ�䡢�жϺ�
*����ֵ��   		��
*/
void UserSetTimer(ZetaTimer_t Timer)
{
	uint8_t temp[4] = {0xff, 0x00, 0x0f, 0x20};
	
	memcpy(ZetaSendBuf.Buf,temp,4);
	
	memcpy(&ZetaSendBuf.Buf[4], &Timer, 11);
		
	ZetaSendBuf.Len = 15;
	
	for(uint8_t i = 0; i < 3; i++)
	{
		ZetaHandle.Send(&ZetaSendBuf);
		
		HAL_Delay(100);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((Payload != Status) && (HAL_GetTick(  ) - overtime < 200));
		
		if(Payload == Status)
			break;
	}
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	memset(ZetaRecviceBuf.Buf, 0, ZetaRecviceBuf.Len);
}

/*UserCloseTimer���ر�Zeta��ʱ��
*������						Timer���жϺ�
*����ֵ��   			��
*/
void UserCloseTimer(ZetaTimer_t Timer)
{
	uint8_t temp[4] = {0xff, 0x00, 0x05, 0x21};
	
	memcpy(ZetaSendBuf.Buf,temp,4);
	
	ZetaSendBuf.Buf[4] = Timer.TimerID;
		
	ZetaSendBuf.Len = 5;
	
	for(uint8_t i = 0; i < 3; i++)
	{
		ZetaHandle.Send(&ZetaSendBuf);
				
		HAL_Delay(100);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((Payload != Status) && (HAL_GetTick(  ) - overtime < 200));
		
		if(Payload == Status)
			break;
	}
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	memset(ZetaRecviceBuf.Buf, 0, ZetaRecviceBuf.Len);
}

/*
 *	UserIntoLowPower:	����͹���ģʽ��ͣ��
 *	����ֵ: 					��
 */
void UserIntoLowPower(void)
{	  
	DEBUG_APP(2,"Goto Sleep Mode");

	User.SleepWakeUp = true;
	BoardDeInitMcu( ); ///�ر�ʱ����
		
	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );

	SET_BIT( PWR->CR, PWR_CR_CWUF );
	/* Set MCU in ULP (Ultra Low Power) */
	HAL_PWREx_EnableUltraLowPower( );

	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp( );
		
	/*****************����ͣ��ģʽ*********************/
	/* Enter Stop Mode */
	__HAL_PWR_CLEAR_FLAG( PWR_FLAG_WU );
	HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/*!
*String_Conversion���ַ���ת��Ϊ16����
*����ֵ: 		    		��
*/
void String_Conversion(char *str, uint8_t *src, uint8_t len)
{
 volatile int i,v;
			
 for(i=0; i<len/2; i++)
 {
	sscanf(str+i*2,"%2X",&v);
	src[i]=(uint8_t)v;
 }
}

void UserReadFlash(void)
{	
	if(FlashRead16(HEART_CYCLE_ADDR)==0||FlashRead16(HEART_CYCLE_ADDR)==0xffff)
	{
		FlashWrite16(HEART_CYCLE_ADDR,&LocationInfor.HeartCycle,1);			
	}

	if(FlashRead16(ALARM_CYCLE_ADDR)==0x00||FlashRead16(ALARM_CYCLE_ADDR)==0xffff)
	{
		if(FlashRead16(ALARM_CYCLE_ADDR) != 0x55aa)
		{
			FlashWrite16(ALARM_CYCLE_ADDR, (uint16_t *)&LocationInfor.AlarmCycle,1);
		}			
	}

	if(FlashRead16(GPS_LOCA_TIME_ADDR)==0||FlashRead16(GPS_LOCA_TIME_ADDR)==0xffff)
	{
		FlashWrite16(GPS_LOCA_TIME_ADDR,&LocationInfor.GpsTime,1);			
	}

	if(FlashRead16(MOVE_CONDITION_ADDR)==0||FlashRead16(MOVE_CONDITION_ADDR)==0xffff)
	{
		FlashWrite16(MOVE_CONDITION_ADDR,(uint16_t *)&LocationInfor.StopTimes,1);			
	}
	
	if(FlashRead16(MOVE_STOP_CONDITION_ADDR)==0||FlashRead16(MOVE_STOP_CONDITION_ADDR)==0xffff)
	{
		FlashWrite16(MOVE_STOP_CONDITION_ADDR,(uint16_t *)&LocationInfor.MoveTimes,1);			
	}

	if(FlashRead16(MOVE_ENABLE_ADDR)==0||FlashRead16(MOVE_ENABLE_ADDR)==0xffff)
	{
		FlashWrite16(MOVE_ENABLE_ADDR,(uint16_t *)&LocationInfor.AlarmEnable,1);			
	}

	if(FlashRead16(MAXLEN_ADDR)==0||FlashRead16(MAXLEN_ADDR)==0xffff)
	{
		ZetaSendBuf.MaxLen = 38;
	}
	else
	{
		ZetaSendBuf.MaxLen = FlashRead16(MAXLEN_ADDR);
	 
		char String_Buffer[2] = {0};
		
		FlashRead16More(MAXLEN_ADDR, (uint16_t *)String_Buffer,	MAXLEN_ADDR_SIZE/2);           
		String_Conversion(String_Buffer, &ZetaSendBuf.MaxLen, MAXLEN_ADDR_SIZE);

		ZetaSendBuf.MaxLen -= FIXLEN;
	} 
	
	if(FlashRead16(MMA8452_DATA_RATE)==0||FlashRead16(MMA8452_DATA_RATE)==0xffff)
	{
		FlashWrite16(MMA8452_DATA_RATE,(uint16_t *)&LocationInfor.Mma8452DaRte,1);
	}
	
	if(FlashRead16(MMA845_MT_COUNT)==0 || FlashRead16(MMA845_MT_COUNT)==0xffff)
	{
		FlashWrite16(MMA845_MT_COUNT,(uint16_t *)&LocationInfor.Mma8452MCount,1);
	}	
	 
	User.SleepTime = FlashRead16(HEART_CYCLE_ADDR);
	
	uint16_t AlarmCycle = FlashRead16(ALARM_CYCLE_ADDR) ;
	
	if(AlarmCycle == 0x55aa)
	{
		LocationInfor.AlarmCycle 	= 0;
	}
	else
	{
		LocationInfor.AlarmCycle 	= AlarmCycle;
	}
	
	LocationInfor.Mma8452DaRte 	=	FlashRead16(MMA8452_DATA_RATE);
	LocationInfor.Mma8452MCount 	=	FlashRead16(MMA845_MT_COUNT);
	LocationInfor.GpsTime 			=	FlashRead16(GPS_LOCA_TIME_ADDR);	
	LocationInfor.StopTimes			= 	FlashRead16(MOVE_CONDITION_ADDR);	
	LocationInfor.MoveTimes 		= 	FlashRead16(MOVE_STOP_CONDITION_ADDR);
	LocationInfor.AlarmEnable  	= 	FlashRead16(MOVE_ENABLE_ADDR);

	DEBUG_APP(2,"mmadate = %02x, mmacout = %02x", LocationInfor.Mma8452DaRte, LocationInfor.Mma8452MCount);	
//	ASLP_RATE_0 | ASLP_RATE_20MS;
}

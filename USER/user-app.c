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

User_t User = {0, 0, false, false};

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

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);	//�˴����ж����ȼ����ܱ�ϵͳʱ�ӵ����ȼ��ߣ����򰴼��������ò�����
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
		BoardEnterStandby(	);
	}		
}

/*UserCheckGps��	�û���ѯGPS��Ϣ
*������						��
*����ֵ��   			��
*/
void UserCheckGps(void)
{						
	if(PATIONNULL == LocatHandles->BreakState(  ))
	{
		Gps.Init(  );
		
		DEBUG_APP(2,"*** Now Start positioning ***"); 
		Gps.Set(  );
	}
}

/*UserLocationVerion	���û���λ������汾
*����									����
*����ֵ								����  
*/
void UserLocationVerion(uint8_t VerCmd)
{
	uint8_t Len = 4;
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
	ZetaSendBuf.Buf[4] = VerCmd;
	
	ZetaSendBuf.Buf[5] = LocationInfor.Versions;
			
	ZetaSendBuf.Buf[2] = Len + 2;
	
	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
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
	uint8_t Len = 4;
	
	memset(ZetaSendBuf.Buf, 0, 20);
	
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
	if(LocatHandles->BreakState(  ) == PATIONDONE)
	{
		memcpy1(&ZetaSendBuf.Buf[Len], LocatHandles->GetLoca( LocatHandles->Buf, LocationCmd ), 8);
		
		ZetaSendBuf.Buf[2] = Len + 8;
	}
	else if(LocatHandles->BreakState(  ) == PATIONFAIL)
	{
		LocationCmd = (LocationCmd)==0x00?(0x0f):++LocationCmd;
		
		ZetaSendBuf.Buf[4] = LocationCmd << 2;
		
		ZetaSendBuf.Buf[2] = 5;
	}
	
	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
	UserSend(&ZetaSendBuf);
			
	/********************�������*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	
	DEBUG_APP(2,"LocationCmd = %d",LocationCmd);
}

uint32_t CurrentSleepTime = 0;

extern uint32_t MMa8452qTime;
extern uint32_t MotionStopTime;

bool 	 MotionBegain = false;

/*UserLocatMotion	���û�����λ���ƶ���Ϣ
*����							����
*����ֵ						����
*/
void UserLocatMotion(void)
{
	if(LocationInfor.AlarmEnable)
	{
		///��Ҫ��ʱ�����˳�������ģʽ		
		DEBUG_APP(2,);
		while(!LocationInfor.BegainLocat && (HAL_GetTick(  ) - MMa8452qTime < (LocationInfor.MoveTimes) * 1000));

		///��δ����������ڶ�ʱ�ϱ��������ϱ�����
		if(MultActive == LocationInfor.MotionState)
		{
			CurrentSleepTime = GetCurrentSleepRtc(  );
								
			DEBUG_APP(2,"AlarmTime = %d ", CurrentSleepTime);
			
			if(10 != CurrentSleepTime)
			{
				SetRtcAlarm(CurrentSleepTime); ///����ʱ��-��ǰʱ��
				UserIntoLowPower(  );
			}
			else ///��������ʱ�䣬ȡ������
			{
				//�ر�RTC����жϣ�������RTCʵ�����
				__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RtcHandle,RTC_IT_WUT);
				__HAL_RTC_TIMESTAMP_DISABLE_IT(&RtcHandle,RTC_IT_TS);
				__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle,RTC_IT_ALRA|RTC_IT_ALRB);
				
				//���RTC����жϱ�־λ
				__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
				__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RtcHandle,RTC_FLAG_TSF); 
				__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle,RTC_FLAG_WUTF);
				
				/* ������л��ѱ�־λ */
				__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
				
				LocatHandles->SetMode( MotionStopMode );	
				return;
			}
		}	
		////���δ���
		else if(Active == LocationInfor.MotionState)
		{
			MotionBegain = true;
			
			if(LocationInfor.BegainLocat)
			{				
				DEBUG_APP(2,);
				LocationInfor.BegainLocat = false;
				
				LocatHandles->SetState(PATIONNULL);	
				
				UserCheckGps(  );			
			}
									
			while(PATIONNULL == LocatHandles->BreakState(  ));
			
			UserSendLocation(ALARM_REP_LOCA_SUCESS);
			
			///�������ٶ�ֹͣ
			if(LocationInfor.MotionState == InActive)
			{
				DEBUG_APP(2,"---- Go to MotionStopMode ----");
				LocatHandles->SetMode( MotionStopMode );
				
				return;
			}
			
			///û���������л������ٶȴ�����ֹͣģʽ						
			LocationInfor.AlarmCycle = FlashRead32(ALARM_CYCLE_ADDR);
			
			if(0 == LocationInfor.AlarmCycle)
			{					
				///���������ٶȻ������������ڷ������������㣬����WaitMode
				LocatHandles->SetMode( WaitMode );	
				
				LocationInfor.MotionState = InActive;
				return;
			}
		 
			User.SleepTime = LocationInfor.AlarmCycle * MINUTE;
			
			DEBUG_APP(2,"LocationInfor.AlarmCycle = %d",LocationInfor.AlarmCycle);
			SetRtcAlarm(User.SleepTime);
			UserIntoLowPower(  );
		}
	}
	else
	{
		LocatHandles->SetMode( WaitMode );
	}
}

/*UserLocatMotionStop	���û�����λ��ֹͣ��Ϣ
*����									����
*����ֵ								����
*/
void UserLocatMotionStop(void)
{
	///��Ҫ��ʱ�����˳�������ģʽ		
	DEBUG_APP(2,);
	while(!LocationInfor.BegainLocat && (HAL_GetTick(  ) - MotionStopTime < (LocationInfor.StopTimes) * 1000));

	DEBUG_APP(2,"--- Mode --- %d MotionState = %d",LocatHandles->GetMode(  ),LocationInfor.MotionState);

	if(MotionMode != LocatHandles->GetMode(  )) 
	{					
		if(LocationInfor.MotionState == InActive)///���ٶȴ���δ��������
		{	
			if(MotionBegain)
			{
				MotionBegain = false;
				DEBUG_APP(2,"---- MotionStopMode ----");
	
				LocatHandles->SetState(PATIONNULL);	
	
				UserCheckGps(  );
	
				while(PATIONNULL == LocatHandles->BreakState(  ));
	
				UserSendLocation(MOVE_STATIC_LOCA_SUCESS); ///�Զ�ֹͣ������ֹͣ����ǰΪ�Զ�ֹͣģʽ
				LocatHandles->SetMode( WaitMode );
			}									
		}		
		
		else if(LocationInfor.MotionState == Invalid)
		{
			CurrentSleepTime = GetCurrentSleepRtc(  );
								
			LocatHandles->SetMode( WaitMode );
			
			DEBUG_APP(2,"AlarmTime = %d ", CurrentSleepTime);

			SetRtcAlarm(CurrentSleepTime); ///����ʱ��-��ǰʱ��
			UserIntoLowPower(  );
		}			
	}		
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
			
			DEBUG_APP(2,"---- waiting %d----", LocationInfor.MotionState);
	
			CurrentSleepTime = GetCurrentSleepRtc(  );
				
			if(10 != CurrentSleepTime && LocationInfor.MotionState == InActive) ///���������������ڣ����ٴ�����
			{
				DEBUG_APP(2,"---- Sleep Again ----");
				
				LocationInfor.MotionState = Invalid;
				SetRtcAlarm(CurrentSleepTime); ///����ʱ��-��ǰʱ��
				UserIntoLowPower(  );
			}		
			
			LocatHandles->SetMode( HeartMode );
			
			break;
		
		case	HeartMode:
			
				while(PATIONNULL == LocatHandles->BreakState(  ));
		
				DEBUG_APP(2,"---- HeartMode ----");
		
				UserSendLocation(HEART_REPORT_SUCESS);
				LocationInfor.HeartCycle = FlashRead16(HEART_CYCLE_ADDR);
		
				LocatHandles->SetMode( WaitMode );
				DEBUG_APP(2,"LocationInfor.HeartCycle = %d",LocationInfor.HeartCycle);
		
				User.SleepTime = LocationInfor.HeartCycle * MINUTE;
		
				SetRtcAlarm(User.SleepTime);
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
		
				LocatHandles->SetMode( WaitMode );
					
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
	
	for(uint8_t i = 0; i < 10; i++)
	{	
		DEBUG_APP(2,"start send data\r\n");
		
		for(uint8_t j = 0; j<SendBuf->Len; j++)
		DEBUG(2,"%02X ",SendBuf->Buf[j]);
		DEBUG(2,"\r\n");
		
		ZetaHandle.Send(SendBuf);
		
		HAL_Delay(100);
		ZetaState_t  Status = ZetaHandle.Recv(  );
		
		uint32_t overtime = HAL_GetTick(  );
		while((DataAck != Status) && (HAL_GetTick(  ) - overtime < 200));
					
		if(DataAck == Status)
		{			
//			HAL_Delay(300);	
//			UserCheckCmd(&UserZetaCheck[NETIME]); ///����ⲿflashʹ��
			break;
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
			HAL_Delay(300);	
		}
		else
		{		
			break;		
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

/*UserDownCommand���������ݴ���
*������			   		 ��
*����ֵ��      		 ��
*/
void UserDownCommand(void)
{
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
//	ZetaHandle.DownCommand(ZetaRecviceBuf.RevBuf);
	
	memcpy1(&ZetaSendBuf.Buf[4], LocatHandles->Cmd(ZetaRecviceBuf.RevBuf), \
	ZetaSendBuf.Len);

	ZetaSendBuf.Buf[2] = 0x04 + ZetaSendBuf.Len; 

	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
	DEBUG_APP(2,"len = %d GetMode = %d\r\n",ZetaSendBuf.Len,LocatHandles->GetMode(  ));
	
	if(QueryLocaMode != LocatHandles->GetMode(  ))
	{
		UserSend(&ZetaSendBuf);
	}

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

	BoardDeInitMcu( ); ///�ر�ʱ����
		
	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );

	SET_BIT( PWR->CR, PWR_CR_CWUF );
	/* Set MCU in ULP (Ultra Low Power) */
	HAL_PWREx_EnableUltraLowPower( );

	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp( );
	
	User.SleepWakeUp = true;
		
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

	if(FlashRead16(ALARM_CYCLE_ADDR)==0||FlashRead16(ALARM_CYCLE_ADDR)==0xffff)
	{
		FlashWrite16(ALARM_CYCLE_ADDR, (uint16_t *)&LocationInfor.AlarmCycle,1);			
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
	 
	User.SleepTime = FlashRead32(HEART_CYCLE_ADDR);

}

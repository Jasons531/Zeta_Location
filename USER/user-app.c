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

User_t User = {0, 0, false, false, false};

static uint8_t DeviceInfo[4] = {0};

/*UserKeyWakeupInit���û��������ѳ�ʼ��
*������							 ��
*����ֵ��   				 ��
*/
void UserKeyWakeupInit(void)
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

/*UserPeriPheralInit���û������ʼ������Դ���ؿ�����Ž��������ʼ��
*������								��
*����ֵ��   					��
*/
void UserPeriPheralInit(void)
{	
	MMA8452WakeupInit(  );
	BoardInitMcu(  );
	
	MMA845xInit(  );
	
	MMA845xID(  );
}


/*UserWakeupHandle��	�û����Ѵ���
*������								��
*����ֵ��   					��
*/
void UserWakeupHandle(void)
{						
	///PA0���ѣ���ʼ������
	if(CheckPowerkey(		))
	{
		DEBUG(2,"���� \r\n");
		
		/* ���������־λ */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RCC_CLEAR_RESET_FLAGS(  );
		
		///��ʼ������
		UserPeriPheralInit(  );
		HAL_Delay(2000);
	}
	else
	{
		HAL_Delay(1000); ///��ֹ��д�쳣
		DEBUG(2,"����\r\n");	
		BoardEnterStandby(	);
	}		
	
	///PC13��mma8452q����
	
	///RTC�����߻���
}

/*UserCheckGps��	�û���ѯGPS��Ϣ
*������						��
*����ֵ��   			��
*/
void UserCheckGps(void)
{						
	Gps.Init(  );
	
	DEBUG_APP(2,"*** Now Start positioning ***"); 
	Gps.Set(  );
	LocatHandles->SetState( PATIONNULL );
}

/*UserSend���û�����Zeta���ͺ�����ע�⣺��������ǰ����ȴ�ģ��ע����ɣ�������ʧ�ܣ�����ģʽĬ�Ͽ�ֱ��ִ��,
*						���������MAX = 49
*������			��
*����ֵ��   ��
*/
void UserSend(Zeta_t *SendBuf)
{
	uint8_t ApplyCounter = 0;
	
	for(uint8_t i = 0; i < 3; i++)
	{	
		DEBUG(2,"start send data\r\n");
		
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
	memcpy1(&ZetaSendBuf.Buf[4], LocatHandles->Cmd(ZetaRecviceBuf.RevBuf), 3);
	
	ZetaSendBuf.Buf[2] = 0x04 + 3; 
	ZetaSendBuf.Len = ZetaSendBuf.Buf[2];
	
	DEBUG_APP(2,"len = %d\r\n",ZetaSendBuf.Len);
	
	memset(ZetaRecviceBuf.RevBuf, 0, strlen((char *)ZetaRecviceBuf.RevBuf));
	UserSend(&ZetaSendBuf);
				
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	memset(ZetaRecviceBuf.RevBuf, 0, sizeof(ZetaRecviceBuf.RevBuf)/sizeof(ZetaRecviceBuf.RevBuf[0])); 	
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
	
	/* ʹ�ܻ������ţ�PC13��Ϊϵͳ�������� */
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2);
	
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
*UserGetAddID��	��ȡ�豸ID
*����ֵ: 		    ��
*/
void UserGetAddID(void)
{
	static char String_Buffer[33] = {0}; ///��ȡflashд���ַ���
	static uint8_t DevTemp[8] = {0};

	FlashRead16More(DEV_ADDR,(uint16_t*)String_Buffer,DEV_ADDR_SIZE/2);         ////DEV
	
//	sscanf(String_Buffer, "%[0-9]", DevTemp);
	
	String_Conversion(String_Buffer, DevTemp, DEV_ADDR_SIZE);  

	///09 07 18 30 0000 0001 ///0730 0001  30: Zeta  31:Zeta+GPS	
	DeviceInfo[0] = DevTemp[1];
	DeviceInfo[1] = DevTemp[3];
	
	DeviceInfo[2] = DevTemp[6];
	DeviceInfo[3] = DevTemp[7];
	
	for(uint8_t i = 0; i < 8; i++)
	DEBUG(2,"%02x ",DevTemp[i]);
	DEBUG(2,"\r\n");

	
	DEBUG(2,"DEV: ");
	for(uint8_t i = 0; i < 4; i++)
	DEBUG(2,"%02x ",DeviceInfo[i]);
	DEBUG(2,"\r\n");
	
	memset(String_Buffer, 33, 0);	
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
	 if(FlashRead32(SLEEP_ADDR)==0||FlashRead32(SLEEP_ADDR)==0xffffffff)
	{
			uint32_t time = 5;//Ĭ��5min
			FlashWrite32(SLEEP_ADDR,&time,1);			
	 }
	
	 if(FlashRead32(AQUATIC_MODE_ADDR)==0||FlashRead32(AQUATIC_MODE_ADDR)==0xffffffff)
	{
			uint32_t data = 1;//Ĭ��ũ��
			FlashWrite32(AQUATIC_MODE_ADDR,&data,1);			
	 }
	
	 if(FlashRead32(MAXLEN_ADDR)==0||FlashRead32(MAXLEN_ADDR)==0xffffffff)
	 {
			ZetaSendBuf.MaxLen = 38;
	 }
	 else
	 {
			ZetaSendBuf.MaxLen = FlashRead16(MAXLEN_ADDR);
		 	DEBUG_APP(2,"ZetaSendBuf.MaxLen = %d",ZetaSendBuf.MaxLen);
		 
			char String_Buffer[2] = {0};
			
			FlashRead16More(MAXLEN_ADDR, (uint16_t *)String_Buffer,	MAXLEN_ADDR_SIZE/2);           
			String_Conversion(String_Buffer, &ZetaSendBuf.MaxLen, MAXLEN_ADDR_SIZE);

		  ZetaSendBuf.MaxLen -= FIXLEN;
		 
			DEBUG_APP(2,"ZetaSendBuf.MaxLen = %d",ZetaSendBuf.MaxLen);
	 }
	 
	User.SleepTime = FlashRead32(SLEEP_ADDR);
			 
	DEBUG_APP(2,"User.SleepTime = %d\r\n",User.SleepTime);

}

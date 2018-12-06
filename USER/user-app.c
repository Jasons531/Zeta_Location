/*
**************************************************************************************************************
*	@file	user-app.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层
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
	{0x10, 1500, Payload}, ///查询mac
	{0x11, 1000, Payload}, ///查询网络时间
	{0x12, 500,  Payload}, ///查询剩发数
	{0x13, 1000, Payload}, ///查询网络质量
};

User_t User = {0, 0, false, false};

static uint8_t DeviceInfo[4] = {0};

/*UserKeyPinInit：用户开机唤醒初始化
*参数：							 无
*返回值：   				 无
*/
void UserKeyPinInit(void)
{						
	GPIO_InitTypeDef GPIO_InitStruct;

	/* 使能(开启)KEY引脚对应IO端口时钟 */  
	__HAL_RCC_GPIOA_CLK_ENABLE(); 

	/* 配置KEY2 GPIO:中断模式，下降沿触发 */
	GPIO_InitStruct.Pin = POWER_KEY;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // 特别注意这里要使用中断模式,下拉，上升沿触发
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(POWER_KEY_IO, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);	//此处的中断优先级不能比系统时钟的优先级高，否则按键消抖就用不了了
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);			//中断使用
}

/*UserKeyWakeupHandle：	用户唤醒处理
*参数：									无
*返回值：   						无
*/
void UserKeyWakeupHandle(void)
{						
	///PA0唤醒：初始化外设
	if(CheckPowerkey(		))
	{		
		/* 清除待机标志位 */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RCC_CLEAR_RESET_FLAGS(  );
		
		///外设初始化
		BoardInitMcu(  );
		DEBUG(2,"开机 \r\n");
		PowerOn(  );
	}
	else
	{
		DEBUG(2,"休眠 \r\n");
		BoardEnterStandby(	);
	}		
}

/*UserCheckGps：	用户查询GPS信息
*参数：						无
*返回值：   			无
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

/*UserLocationVerion	：用户定位器软件版本
*参数									：无
*返回值								：无  
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
			
	/********************缓存清除*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

/*UserSendLocation	：用户获取定位器信息
*参数								：无
*返回值							：无
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
			
	/********************缓存清除*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
	
	DEBUG_APP(2,"LocationCmd = %d",LocationCmd);
}

uint32_t CurrentSleepTime = 0;

extern uint32_t MMa8452qTime;
extern uint32_t MotionStopTime;

bool 	 MotionBegain = false;

/*UserLocatMotion	：用户处理定位器移动信息
*参数							：无
*返回值						：无
*/
void UserLocatMotion(void)
{
	if(LocationInfor.AlarmEnable)
	{
		///需要超时机制退出到心跳模式		
		DEBUG_APP(2,);
		while(!LocationInfor.BegainLocat && (HAL_GetTick(  ) - MMa8452qTime < (LocationInfor.MoveTimes) * 1000));

		///多次触发则周期内定时上报：周期上报数据
		if(MultActive == LocationInfor.MotionState)
		{
			CurrentSleepTime = GetCurrentSleepRtc(  );
								
			DEBUG_APP(2,"AlarmTime = %d ", CurrentSleepTime);
			
			if(10 != CurrentSleepTime)
			{
				SetRtcAlarm(CurrentSleepTime); ///闹钟时间-当前时间
				UserIntoLowPower(  );
			}
			else ///到达休眠时间，取消休眠
			{
				//关闭RTC相关中断，可能在RTC实验打开了
				__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RtcHandle,RTC_IT_WUT);
				__HAL_RTC_TIMESTAMP_DISABLE_IT(&RtcHandle,RTC_IT_TS);
				__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle,RTC_IT_ALRA|RTC_IT_ALRB);
				
				//清除RTC相关中断标志位
				__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
				__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RtcHandle,RTC_FLAG_TSF); 
				__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle,RTC_FLAG_WUTF);
				
				/* 清除所有唤醒标志位 */
				__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
				
				LocatHandles->SetMode( MotionStopMode );	
				return;
			}
		}	
		////单次触发
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
			
			///侦听加速度停止
			if(LocationInfor.MotionState == InActive)
			{
				DEBUG_APP(2,"---- Go to MotionStopMode ----");
				LocatHandles->SetMode( MotionStopMode );
				
				return;
			}
			
			///没触发，则切换进加速度传感器停止模式						
			LocationInfor.AlarmCycle = FlashRead32(ALARM_CYCLE_ADDR);
			
			if(0 == LocationInfor.AlarmCycle)
			{					
				///侦听到加速度还触发：但周期发送条件不满足，进入WaitMode
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

/*UserLocatMotionStop	：用户处理定位器停止信息
*参数									：无
*返回值								：无
*/
void UserLocatMotionStop(void)
{
	///需要超时机制退出到心跳模式		
	DEBUG_APP(2,);
	while(!LocationInfor.BegainLocat && (HAL_GetTick(  ) - MotionStopTime < (LocationInfor.StopTimes) * 1000));

	DEBUG_APP(2,"--- Mode --- %d MotionState = %d",LocatHandles->GetMode(  ),LocationInfor.MotionState);

	if(MotionMode != LocatHandles->GetMode(  )) 
	{					
		if(LocationInfor.MotionState == InActive)///加速度触发未满足条件
		{	
			if(MotionBegain)
			{
				MotionBegain = false;
				DEBUG_APP(2,"---- MotionStopMode ----");
	
				LocatHandles->SetState(PATIONNULL);	
	
				UserCheckGps(  );
	
				while(PATIONNULL == LocatHandles->BreakState(  ));
	
				UserSendLocation(MOVE_STATIC_LOCA_SUCESS); ///自动停止、命令停止，当前为自动停止模式
				LocatHandles->SetMode( WaitMode );
			}									
		}		
		
		else if(LocationInfor.MotionState == Invalid)
		{
			CurrentSleepTime = GetCurrentSleepRtc(  );
								
			LocatHandles->SetMode( WaitMode );
			
			DEBUG_APP(2,"AlarmTime = %d ", CurrentSleepTime);

			SetRtcAlarm(CurrentSleepTime); ///闹钟时间-当前时间
			UserIntoLowPower(  );
		}			
	}		
}

/*UserLocatReport	：用户定位器信息上报
*参数							：无
*返回值						：无
*/
void UserLocatReport(void)
{
	switch(LocatHandles->GetMode(  ))
	{
		case WaitMode:
			
			DEBUG_APP(2,"---- waiting %d----", LocationInfor.MotionState);
	
			CurrentSleepTime = GetCurrentSleepRtc(  );
				
			if(10 != CurrentSleepTime && LocationInfor.MotionState == InActive) ///在心跳休眠周期内，则再次休眠
			{
				DEBUG_APP(2,"---- Sleep Again ----");
				
				LocationInfor.MotionState = Invalid;
				SetRtcAlarm(CurrentSleepTime); ///闹钟时间-当前时间
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

/*UserSend：用户调用Zeta发送函数：注意：发送数据前必须等待模块注册完成，否则发送失败，其它模式默认可直接执行,
*						最大发送数据MAX = 49
*参数：			无
*返回值：   无
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
//			UserCheckCmd(&UserZetaCheck[NETIME]); ///结合外部flash使用
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

/*UserSendTest：测试Zeta发送数据
*参数：			    无
*返回值：       无
*/
void UserSendTest(void)
{
	uint8_t len= 3;
	volatile uint16_t	UpSeqCounter = 1; 
	
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
		
	ZetaSendBuf.Buf[0] = 0xff;
	ZetaSendBuf.Buf[1] = 0x00;
	
	ZetaSendBuf.Buf[3] = 0x02;
	
	ZetaSendBuf.Buf[4] = (0x01 << 4); ///|充电状态
	
	/********************设备ID*****************/
	memcpy(&ZetaSendBuf.Buf[5], &DeviceInfo[0], 4); 
	
	ZetaSendBuf.Buf[9] = 0x11; //帧数
			
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
		
	/********************缓存清除*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

/*UserDownCommand：下行数据处理
*参数：			   		 无
*返回值：      		 无
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

/*UserCheckCmd：用户查询Zeta：服务器查询下发
*参数：					UserZetaCheckCmd：查询命令
*返回值：   		无
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

/*UserSetHeart：用户设置Zeta心跳
*参数：					mode: 0--正常模式6H一次心跳，1--测试模式10S一次心跳
*返回值：   		无
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

/*UserSetTimer：设置Zeta定时器时间
*参数：					Timer：设置中断开始时间、间隔时间、中断号
*返回值：   		无
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

/*UserCloseTimer：关闭Zeta定时器
*参数：						Timer：中断号
*返回值：   			无
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
 *	UserIntoLowPower:	进入低功耗模式：停机
 *	返回值: 					无
 */
void UserIntoLowPower(void)
{	  
	DEBUG_APP(2,"Goto Sleep Mode");

	BoardDeInitMcu( ); ///关闭时钟线
		
	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );

	SET_BIT( PWR->CR, PWR_CR_CWUF );
	/* Set MCU in ULP (Ultra Low Power) */
	HAL_PWREx_EnableUltraLowPower( );

	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp( );
	
	User.SleepWakeUp = true;
		
	/*****************进入停机模式*********************/
	/* Enter Stop Mode */
	__HAL_PWR_CLEAR_FLAG( PWR_FLAG_WU );
	HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/*!
*String_Conversion：字符串转换为16进制
*返回值: 		    		无
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

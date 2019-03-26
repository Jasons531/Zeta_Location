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

User_t User = {0, 1, 1, 0, 0, false, false};

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

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 8, 0);	//此处的中断优先级不能比系统时钟的优先级高，否则按键消抖就用不了了
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
		
		/********配置加速度传感器休眠*********/
		MX_I2C2_Init(  );				
		MMA845xStandby(  );
		
		BoardEnterStandby(  );
	}		
}

/*UserCheckGps：	用户查询GPS信息
*参数：				无
*返回值：   		无
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

/*UserLocationVerion	：用户定位器软件版本
*参数									：无
*返回值								：无  
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
			
	/********************缓存清除*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

/*UserSendLocation	：用户获取定位器信息
*参数								：无
*返回值							：无
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
			
	/********************缓存清除*******************/
	memset(ZetaSendBuf.Buf, 0, ZetaSendBuf.Len);
}

int32_t CurrentSleepTime = 0;

/*UserLocatMotion	：用户处理定位器移动信息
*参数					：无
*返回值				：无
*/
void UserLocatMotion(void)
{
	uint16_t AlarmCycle = 0;
	
	if(LocationInfor.AlarmEnable && !LocationInfor.SingleAlarm)
	{		
		DEBUG_APP(3,"LocationInfor.SingleAlarm = %d MotionState = %d",LocationInfor.SingleAlarm, LocationInfor.MotionState);

		////单次触发
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
				
			///侦听到加速度还触发：但周期发送条件不满足，则记录当前休眠时间：1.若休眠时间到达则启动心跳模式，心跳完成后设置闹钟时间
			///但不休眠，重新等待振动停止后才进入WaitMode休眠模式
			///2.若休眠时间没到达，则等待振动停止，停止后进入WaitMode(开启定时器侦听休眠时间)，进入休眠需要启动进入休眠标志位
			if(0 == LocationInfor.AlarmCycle)
			{					
				///以下操作需要存放于定时器中进行实时侦听
				LocationInfor.SingleAlarm = true;
			}	
			else
			{
				///加速度一直触发，同时周期上报，则开启循环定时器	
				LocationInfor.MotionState = MultActive; ///定位器一直触发状态，等待周期性上报状态
								
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

/*UserLocatMotionStop	：用户处理定位器停止信息
*参数							：无
*返回值						：无
*/
void UserLocatMotionStop(void)
{	
	DEBUG_APP(2,"--- Mode --- %d MotionState = %d",LocatHandles->GetMode(  ),LocationInfor.MotionState);

	if(LocationInfor.MotionState == StopActive)///加速度停止
	{	
		DEBUG_APP(2,"---- MotionStopMode ----");
		
		LocationInfor.SingleAlarm  = false;
		LocationInfor.MotionHandle = false;
		
		LocatHandles->SetState(PATIONNULL);	

		UserCheckGps(  );

		while(PATIONNULL == LocatHandles->BreakState(  ));

		UserSendLocation(MOVE_STATIC_LOCA_SUCESS); ///自动停止、命令停止，当前为自动停止模式
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

/*UserLocatDetect			：侦听触发侦听
*参数							：无
*返回值						：无
*/
void UserLocatDetect(void)
{
	uint32_t TimeOut = 0;
	
	///移动模式：移动处理
	if(MotionMode == LocatHandles->GetMode(  )) 
	{
		if(LocationInfor.MotionHandle)
		{
			TimeOut = LocationInfor.StopTimes;
			
			TimeOut *= 1000;
			
			///移动已执行则侦听停止振动条件
			if(HAL_GetTick(  ) - LocationInfor.CollectMoveStopTime > TimeOut)
			{
				LocatHandles->SetMode( MotionStopMode );

				LocationInfor.MotionState = StopActive;
				DEBUG_APP(2,"*** StopTimeOut:%d ***",TimeOut/1000);
				
				return;
			}
			
			///等待移动停止判断心跳周期是否到达：
			if((0 == GetCurrentSleepRtc(  )) && (User.AlarmDate == User.CurrentDate))
			{		
				DEBUG_APP(2,"*** CollectMoveStopTime:%d ***",(HAL_GetTick(  ) - LocationInfor.CollectMoveStopTime)/1000);
				LocatHandles->SetMode( HeartMode );	
				
				LocatHandles->SetState( PATIONNULL );
				
				return;
			}
		}
	}	
	else if(WaitMode == LocatHandles->GetMode(  ) || HeartMode == LocatHandles->GetMode(  )) ///异常保护机制：侦听触发，但未满足触发条件
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
	
	///周期上报报警：加速度一直触发，同时心跳周期没触发
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
	else ///运动过程接收到关闭报警，立即切换到停止模式
	{
		if(LocationInfor.MotionHandle)
		{
			LocatHandles->SetMode( MotionStopMode );
			
			LocationInfor.MotionState = StopActive;
		}
	}
	
	User.TimerCounter ++;
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
			
			if(LocationInfor.HeartArrive)
			{
				LocatHandles->SetMode( HeartMode );
				
				break;
			}
				
			if(LocationInfor.MotionState == StopActive || LocationInfor.MotionState == FailActive) ///在心跳休眠周期内，则再次休眠
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
					else if(User.AlarmDate == User.CurrentDate)//休眠时间到达
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
				
		  ////移动模式等待移动停止，切换回移动模式
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
			
			if(SingleActive != LocationInfor.MotionState || MultActive != LocationInfor.MotionState) ///待测试
			{
				LocationInfor.MotionState = StopActive;
				LocatHandles->SetMode( WaitMode );
			}	
					
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
//			UserCheckCmd(&UserZetaCheck[NETIME]); ///结合外部flash使用
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


bool setheart = false;
/*UserDownCommand：下行数据处理
*参数：			    无
*返回值：      	 无
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

	User.SleepWakeUp = true;
	BoardDeInitMcu( ); ///关闭时钟线
		
	// Disable the Power Voltage Detector
	HAL_PWR_DisablePVD( );

	SET_BIT( PWR->CR, PWR_CR_CWUF );
	/* Set MCU in ULP (Ultra Low Power) */
	HAL_PWREx_EnableUltraLowPower( );

	/* Enable the fast wake up from Ultra low power mode */
	HAL_PWREx_EnableFastWakeUp( );
		
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

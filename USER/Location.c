/*
**************************************************************************************************************
*	@file		Location.c
*	@author Jason
*	@version 
*	@date    
*	@brief	资产定位器处理文件
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/

#include <string.h>
#include "Location.h"
#include "debug.h"
#include "gps.h"

LocationIn_t LocationInfor = {false, VERSIOS, 12*HOUR, 5, 2*MINUTE, 5, 5, 1, HeartMode, InActive}; //30-2*MINUTE

LocatH_t 	LocatHandle;

LocatH_t 	*LocatHandles;

/*LocationInit：定位器初始化
*参数					：无
*返回					：无
*/
void LocationInit( void )
{
	LocatHandle.Cmd				 = LocationCmd;
	LocatHandle.GetLoca		 = GetLocation;
	LocatHandle.CheckGps 	 = LocationCheckGps;
	LocatHandle.SetState	 = LocationSetState;
	LocatHandle.BreakState = LocationBreakState;
	LocatHandle.SetMode		 = LocationSetMode;
	LocatHandle.GetMode		 = LocationGetMode;
	
	LocatHandles = &LocatHandle;
	
	LocatHandles->SetState(PATIONNULL);
	
	LocatHandles->SetMode(HeartMode);
}

/*LocationCmd：定位器下行命令处理
*参数		 ：Zeta_t
*返回		 ：处理缓存区，提供zeta应答
*/
uint8_t *LocationCmd(uint8_t *ZRev)
{
	uint8_t	*HeartBuf = ZRev;
	
	ZetaSendBuf.Len = 0;
		
	HeartBuf[ZetaSendBuf.Len++] = ZRev[0];
	
	switch(ZRev[0])
	{
		case HEART_SET_CYCLE: ///设置心跳周期
			LocationInfor.HeartCycle 	= (ZRev[ZetaSendBuf.Len++] << 8);
			LocationInfor.HeartCycle |= ZRev[ZetaSendBuf.Len++];
			
			DEBUG_APP(2,"HeartCycle = %04X %02X %02X %02X\r\n",LocationInfor.HeartCycle, HeartBuf[0],HeartBuf[1],HeartBuf[2]);
			///保存flash
			FlashWrite16(HEART_CYCLE_ADDR,&LocationInfor.HeartCycle,1);	
			
		break;
		
		case HEART_CHECK_CYCLE: ///查询心跳
		
			LocationInfor.HeartCycle 		= FlashRead16(HEART_CYCLE_ADDR);;			
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 0)&0xff;

			DEBUG_APP(2,"HeartCycle = %04X %02X %02X\r\n",LocationInfor.HeartCycle, HeartBuf[1],HeartBuf[2]);
						
		break;
		
		case ALARM_SET_CYCLE:	///设置告警周期
			LocationInfor.AlarmCycle 		= ZRev[ZetaSendBuf.Len++];
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmCycle;
			
			///保存flash
			FlashWrite16(ALARM_CYCLE_ADDR, (uint16_t *)&LocationInfor.AlarmCycle,1);	
		
		break;
		
		case ALARM_CHECK_CYCLE: ///查询告警
			
			LocationInfor.AlarmCycle		= FlashRead16(ALARM_CYCLE_ADDR);		

			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.AlarmCycle >> 0)&0xff;
		
		break;
		
		case GPS_SET_LOCA_TIME: ///定位超时时间
			LocationInfor.GpsTime = ZRev[ZetaSendBuf.Len++] << 8;
			LocationInfor.GpsTime |= ZRev[ZetaSendBuf.Len++] << 0;	
			
			DEBUG_APP(2,"LocationInfor.GpsTime = %04X %02X %02X\r\n",LocationInfor.GpsTime, HeartBuf[1],HeartBuf[2]);
			
			///保存flash
			FlashWrite16(GPS_LOCA_TIME_ADDR,&LocationInfor.GpsTime,1);	
			
		break;
		
		case GPS_CHECK_LOCA_TIME: 
			
			LocationInfor.GpsTime = FlashRead16(GPS_LOCA_TIME_ADDR);
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.GpsTime >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.GpsTime >> 0)&0xff;
			
			DEBUG_APP(2,"LocationInfor.GpsTime = %04X %02X %02X\r\n",LocationInfor.GpsTime, HeartBuf[1],HeartBuf[2]);
			
		break;
		
		case MOVE_SET_MOVE_CONDITION: ///设置移动判定时间
			LocationInfor.MoveTimes 		= ZRev[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.MoveTimes;
			
			///保存flash
			FlashWrite16(MOVE_CONDITION_ADDR,(uint16_t *)&LocationInfor.MoveTimes,1);	
			
		break;
		
		case MOVE_CHECK_MOVE_CONDITION:
			
			LocationInfor.MoveTimes 		= FlashRead16(MOVE_CONDITION_ADDR);

			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.MoveTimes;
		
		break;
		
		case MOVE_SET_STOP_CONDITION: ///设置停止移动判定时间
			LocationInfor.StopTimes 		= ZRev[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.StopTimes;
		
			///保存flash
			FlashWrite16(MOVE_STOP_CONDITION_ADDR,(uint16_t *)&LocationInfor.StopTimes,1);
	
		break;
		
		case MOVE_CHECK_STOP_CONDITION:
		
			LocationInfor.StopTimes			= FlashRead16(MOVE_STOP_CONDITION_ADDR);		
			
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.StopTimes;	
		break;
		
		case MOVE_SET_MOVE_ENABLE: ///设置告警开关
			LocationInfor.AlarmEnable 	= ZRev[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.AlarmEnable;
			
			///保存flash
			FlashWrite16(MOVE_ENABLE_ADDR,(uint16_t *)&LocationInfor.AlarmEnable,1);	
	
		break;
		
		case MOVE_CHECK_MOVE_ENABLE:
			
			LocationInfor.AlarmEnable 	= FlashRead16(MOVE_ENABLE_ADDR);		

			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmEnable;
	
		break;
		
		case QUERY_CHECK_VER:
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.Versions;	
		break;
		
		case QUERY_DEV_LOCA:
		///启动反馈命令: QUERY_FEED_BACK		
//		if(LocatHandles->BreakState(  ) == PATIONDONE)
//		{			
//			memcpy1(&HeartBuf[ZetaSendBuf.Len], LocatHandles->GetLoca( LocatHandles->Buf, QUERY_FEED_LOCA_SUCESS ), 8);
//						
//			ZetaSendBuf.Len += 8;
//		}
//		else if(LocatHandles->BreakState(  ) == PATIONFAIL)
//		{			
//			HeartBuf[ZetaSendBuf.Len++] = QUERY_FEED_LOCA_FAIL << 2;
//		}		
			LocatHandles->SetMode( QueryLocaMode );
		
			DEBUG_APP(2,"QUERY_DEV_LOCA = %d",QUERY_DEV_LOCA);

		break;
		
		case QUERY_OSC_STATE: ///加速度传感器数值
		///HeartBuf[1] = 振动开关值(实际指哪?);	
			
		break;
		
		case QUERY_DEV_INFOR:
//		HeartBuf[ZetaSendBuf.Len++] = 初始振动开关数值
//		HeartBuf[ZetaSendBuf.Len++]
		
			LocationInfor.HeartCycle 		= FlashRead16(HEART_CYCLE_ADDR);

			LocationInfor.AlarmCycle 		= FlashRead16(ALARM_CYCLE_ADDR);
			
			LocationInfor.AlarmEnable 	= FlashRead16(MOVE_ENABLE_ADDR);		
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 0)&0xff;
			
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmCycle;
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmEnable;
		
		break;
		
		default:
			
		break;
	}
	
	return HeartBuf;
}

/*GetLocation：定位器下行命令处理
*参数				 : GpsLocation 、LocationCmd：下行命令
*返回				 ：处理缓存区，提供zeta应答
*/
uint8_t *GetLocation(char *GpsLocation, uint8_t LocationCmd)
{
	char		GPLL[10] = {0};
	char 		NorthLat[15] = {0};
	char 		EastLot[15] = {0};

	char 		Nstate = 0;
	char 		Estate = 0;

	double 	data_N = 0;
	double 	data_E = 0;
	
	uint8_t len = 0;
	
	uint8_t BufTemp[8] = {0};
	
	uint8_t *GpsBuf = BufTemp; ///赋地址，直接传递会出问题

	sscanf(GpsLocation, "%[^,]%*[,] %[^,]%*[,] %[^,]%*[,] %[^,]%*[,] %[^,]%*[,]", GPLL, \
	NorthLat, &Nstate, EastLot, &Estate);     ////取数到,截止，同时过滤,

	DEBUG_APP(2,"The lowercase is: %s %s%c %s%c\r\n", GPLL, NorthLat, Nstate, EastLot, Estate);
	
	if('N' == Nstate)
	{
		SetGpsMode.South = false;
	}
	else if('S' == Nstate)
	{
		SetGpsMode.South = true;
	}
	
	if('E' == Estate)
	{
		SetGpsMode.West = false;
	}
	else if('W' == Estate)
	{
		SetGpsMode.West = true;
	}

	sscanf(NorthLat, "%lf", &data_N);
	sscanf(EastLot, "%lf", &data_E);
	
	SetGpsMode.NorthSpend = data_N * 10000;
	SetGpsMode.EastSpend = data_E * 10000;
	
	DEBUG_APP(3,"LocationCmd = %02x %02x %02x %02x",LocationCmd, (LocationCmd << 2), (SetGpsMode.West << 1),(SetGpsMode.South << 0));
	
	BufTemp[len++] = (LocationCmd << 2) | (SetGpsMode.West << 1) | (SetGpsMode.South << 0);
						
	BufTemp[len++] = (SetGpsMode.EastSpend >> 20)&0xFF; ///28bit取高8bit，注意非32bit取运算
	BufTemp[len++] = (SetGpsMode.EastSpend >> 12)&0xFF;
	BufTemp[len++] = (SetGpsMode.EastSpend >> 4)&0xFF;
	
	BufTemp[len++] = (((SetGpsMode.EastSpend >> 0) & 0xF) << 4) | ((SetGpsMode.NorthSpend >> 24) & 0xF); ///28bit取高4bit
	
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 16) & 0xFF;
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 8) & 0xFF;
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 0) & 0xFF;	
	
	DEBUG_APP(2,"E_Data = %.4f N_Data = %.4f %d %d\r\n",data_N, data_E, SetGpsMode.NorthSpend, SetGpsMode.EastSpend);
	
	return GpsBuf;
}

/*LocationCheck：定时扫描GPS定位信息
*参数				 	 : Location_t：超时时间
*返回				   ：无
*/
void LocationCheckGps(LocationIn_t Locat)
{
	if(SetGpsMode.Gpll)
	{	
		DEBUG_APP(2,"Locat.GpsTime = %d",Locat.GpsTime);
		if( ((HAL_GetTick( ) - SetGpsMode.GpsOverTime) > Locat.GpsTime * MSEC) )  ///GPS 2分钟内定位失败，默认GPS异常不再定位 && (LocatHandles->BreakState(  ) == PATIONNULL)
		{	 
			DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsMode.GpsOverTime);

			LocatHandles->SetState( PATIONFAIL );

			Gps.Disable(  ); ///关闭GPS

			SetGpsMode.Start = false;
			SetGpsMode.Gpll = false;

			memset(LocatHandles->Buf, 0, strlen(LocatHandles->Buf));

			gpsx.gpssta = 0;	
		}
	}
}

/*LocationSetState：设置GPS定位状态
*参数				 	 		: State
*返回				   		：无
*/
void LocationSetState(uint8_t State)
{
	SetGpsMode.LocationState = State;
}

/*LocationBreakState：获取GPS定位状态
*参数				 	 			: 无
*返回				   			：State
*/
uint8_t LocationBreakState( void )
{
	return SetGpsMode.LocationState;
}

/*LocationSetMode	：设置定位器工作模式
*参数				 	 		: 定位器工作模式
*返回				   		：无
*/
void LocationSetMode( Locatmode_t Mode )
{
	LocationInfor.Mode = Mode;
}

/*LocationGetMode		：获取定位器工作模式
*参数				 	 			: 无
*返回				   			：State
*/
Locatmode_t LocationGetMode( void )
{
	return LocationInfor.Mode;
}

/*
*memcpy1：	数据拷贝
*dst：			拷贝目标
*src:				拷贝源地址
*size:			数据大小
*返回：			无
*/
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
	while( size-- )
	{
		*dst++ = *src++;
	}
}

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

#define 				VERSIOS							0x01
#define					MSEC								1000
#define					MINUTE							60
#define 				HOUR								3600

LocationIn_t LocationInfor = {false, 12*HOUR, 5, 0, 0.5*MINUTE, 5, 5, 1, VERSIOS, HeartMode, Wait}; //30-2*MINUTE

LocatH_t 	LocatHandle;

LocatH_t *LocatHandles;

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
		
	switch(ZRev[0])
	{
		case HEART_SET_CYCLE: ///设置心跳周期
		LocationInfor.HeartCycle = (ZRev[1] << 8);
		LocationInfor.HeartCycle |= ZRev[2];
		memcpy1(HeartBuf, &ZRev[0], 3);
		
		DEBUG_APP(2,"HeartCycle = %04x %02x %02x %02x\r\n",LocationInfor.HeartCycle, HeartBuf[0],HeartBuf[1],HeartBuf[2]);
		///保存flash
		FlashWrite16(HEART_CYCLE_ADDR,&LocationInfor.HeartCycle,1);	
			
		break;
		
		case HEART_CHECK_CYCLE: ///查询心跳
		HeartBuf[1] = (LocationInfor.HeartCycle >> 8)&0xff;
		HeartBuf[2] = (LocationInfor.HeartCycle >> 0)&0xff;

		DEBUG_APP(2,"HeartCycle = %04x %02x %02x\r\n",LocationInfor.HeartCycle, HeartBuf[1],HeartBuf[2]);
						
		break;
		
		case ALARM_SET_CYCLE:	///设置告警周期
		LocationInfor.AlarmCycle = ZRev[1];
		memcpy1(HeartBuf, ZRev, strlen((char *)ZRev));
		
		///保存flash
		FlashWrite16(ALARM_CYCLE_ADDR, (uint16_t *)&LocationInfor.AlarmCycle,1);	
		
		break;
		
		case ALARM_CHECK_CYCLE: ///查询告警
		HeartBuf[1] = (LocationInfor.AlarmCycle >> 0)&0xff;
		
		break;
		
		case GPS_SET_LOCA_TIME: ///定位超时时间
		LocationInfor.GpsTime = ZRev[1] << 8;
		LocationInfor.GpsTime |= ZRev[2] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///保存flash
		FlashWrite16(GPS_LOCA_TIME_ADDR,&LocationInfor.GpsTime,1);	
			
		break;
		
		case GPS_CHECK_LOCA_TIME: 
		HeartBuf[1] = (LocationInfor.GpsTime >> 8)&0xff;
		HeartBuf[2] = (LocationInfor.GpsTime >> 0)&0xff;
			
		break;
		
		case MOVE_SET_MOVE_CONDITION: ///设置移动判定时间
		LocationInfor.MoveTimes = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///保存flash
		FlashWrite16(MOVE_CONDITION_ADDR,(uint16_t *)&LocationInfor.StopTimes,1);	
			
		break;
		
		case MOVE_CHECK_MOVE_CONDITION:
		HeartBuf[1] = LocationInfor.MoveTimes;
		
		break;
		
		case MOVE_SET_STOP_CONDITION: ///设置停止移动判定时间
		LocationInfor.StopTimes = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///保存flash
		FlashWrite16(MOVE_STOP_CONDITION_ADDR,(uint16_t *)&LocationInfor.MoveTimes,1);
	
		break;
		
		case MOVE_CHECK_STOP_CONDITION:
		HeartBuf[1] = LocationInfor.StopTimes;	
		break;
		
		case MOVE_SET_MOVE_ENABLE: ///设置告警开关
		LocationInfor.AlarmEnable = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///保存flash
		FlashWrite16(MOVE_ENABLE_ADDR,(uint16_t *)&LocationInfor.AlarmEnable,1);	
	
		break;
		
		case MOVE_CHECK_MOVE_ENABLE:
		HeartBuf[1] = LocationInfor.AlarmEnable;
	
		break;
		
		case QUERY_CHECK_VER:
		HeartBuf[1] = LocationInfor.Versions;	
		break;
		
		case QUERY_DEV_LOCA:
			///启动反馈命令: QUERY_FEED_BACK
		
		break;
		
		case QUERY_OSC_STATE: ///加速度传感器数值
		///HeartBuf[1] = 振动开关值(实际指哪?);	
			
		break;
		
		case QUERY_DEV_INFOR:
//		HeartBuf[1] = 初始振动开关数值
//		HeartBuf[2]
		
		HeartBuf[3] = (LocationInfor.HeartCycle >> 8)&0xff;
		HeartBuf[4] = (LocationInfor.HeartCycle >> 0)&0xff;
		
		HeartBuf[5] = LocationInfor.AlarmCycle;
		HeartBuf[6] = LocationInfor.AlarmEnable;
		
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
	char GPLL[10] = {0};
	char NorthLat[15] = {0};
	char EastLot[15] = {0};

	char Nstate = 0;
	char Estate = 0;

	double data_N = 0;
	double data_E = 0;
	
	uint8_t len = 0;
	
	uint8_t *GpsBuf;

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
	
	GpsBuf[len++] = (LocationCmd << 2) | (SetGpsMode.West << 1) | (SetGpsMode.South << 0);
					
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 20)&0xFF; ///28bit取高8bit，注意非32bit取运算
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 12)&0xFF;
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 4)&0xFF;
	
	GpsBuf[len++] = (((SetGpsMode.EastSpend >> 0) & 0xF) << 4) | ((SetGpsMode.NorthSpend >> 24) & 0xF); ///28bit取高4bit
	
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 16) & 0xFF;
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 8) & 0xFF;
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 0) & 0xFF;	
	
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
		if(((HAL_GetTick( ) - SetGpsMode.GpsOverTime) > Locat.GpsTime * MSEC) && (LocatHandles->BreakState(  ) == PATIONNULL))  ///GPS 2分钟内定位失败，默认GPS异常不再定位
	 {	 
			DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsMode.GpsOverTime);
			 
			LocatHandles->SetState( PATIONFAIL );
		
			Gps.Disable(  ); ///关闭GPS
			 
			SetGpsMode.Start = false;
		  SetGpsMode.Gpll = false;

			memset(LocatHandles->Buf, 0, strlen(LocatHandles->Buf));
		 
			gpsx.gpssta = 0;	
						 
//			HAL_TIM_Base_Stop_IT(&htim2);
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

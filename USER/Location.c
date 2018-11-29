/*
**************************************************************************************************************
*	@file		Location.c
*	@author Jason
*	@version 
*	@date    
*	@brief	�ʲ���λ�������ļ�
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

/*LocationInit����λ����ʼ��
*����					����
*����					����
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

/*LocationCmd����λ�����������
*����		 ��Zeta_t
*����		 �������������ṩzetaӦ��
*/
uint8_t *LocationCmd(uint8_t *ZRev)
{
	uint8_t	*HeartBuf = ZRev;
		
	switch(ZRev[0])
	{
		case HEART_SET_CYCLE: ///������������
		LocationInfor.HeartCycle = (ZRev[1] << 8);
		LocationInfor.HeartCycle |= ZRev[2];
		memcpy1(HeartBuf, &ZRev[0], 3);
		
		DEBUG_APP(2,"HeartCycle = %04x %02x %02x %02x\r\n",LocationInfor.HeartCycle, HeartBuf[0],HeartBuf[1],HeartBuf[2]);
		///����flash
		FlashWrite16(HEART_CYCLE_ADDR,&LocationInfor.HeartCycle,1);	
			
		break;
		
		case HEART_CHECK_CYCLE: ///��ѯ����
		HeartBuf[1] = (LocationInfor.HeartCycle >> 8)&0xff;
		HeartBuf[2] = (LocationInfor.HeartCycle >> 0)&0xff;

		DEBUG_APP(2,"HeartCycle = %04x %02x %02x\r\n",LocationInfor.HeartCycle, HeartBuf[1],HeartBuf[2]);
						
		break;
		
		case ALARM_SET_CYCLE:	///���ø澯����
		LocationInfor.AlarmCycle = ZRev[1];
		memcpy1(HeartBuf, ZRev, strlen((char *)ZRev));
		
		///����flash
		FlashWrite16(ALARM_CYCLE_ADDR, (uint16_t *)&LocationInfor.AlarmCycle,1);	
		
		break;
		
		case ALARM_CHECK_CYCLE: ///��ѯ�澯
		HeartBuf[1] = (LocationInfor.AlarmCycle >> 0)&0xff;
		
		break;
		
		case GPS_SET_LOCA_TIME: ///��λ��ʱʱ��
		LocationInfor.GpsTime = ZRev[1] << 8;
		LocationInfor.GpsTime |= ZRev[2] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///����flash
		FlashWrite16(GPS_LOCA_TIME_ADDR,&LocationInfor.GpsTime,1);	
			
		break;
		
		case GPS_CHECK_LOCA_TIME: 
		HeartBuf[1] = (LocationInfor.GpsTime >> 8)&0xff;
		HeartBuf[2] = (LocationInfor.GpsTime >> 0)&0xff;
			
		break;
		
		case MOVE_SET_MOVE_CONDITION: ///�����ƶ��ж�ʱ��
		LocationInfor.MoveTimes = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///����flash
		FlashWrite16(MOVE_CONDITION_ADDR,(uint16_t *)&LocationInfor.StopTimes,1);	
			
		break;
		
		case MOVE_CHECK_MOVE_CONDITION:
		HeartBuf[1] = LocationInfor.MoveTimes;
		
		break;
		
		case MOVE_SET_STOP_CONDITION: ///����ֹͣ�ƶ��ж�ʱ��
		LocationInfor.StopTimes = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///����flash
		FlashWrite16(MOVE_STOP_CONDITION_ADDR,(uint16_t *)&LocationInfor.MoveTimes,1);
	
		break;
		
		case MOVE_CHECK_STOP_CONDITION:
		HeartBuf[1] = LocationInfor.StopTimes;	
		break;
		
		case MOVE_SET_MOVE_ENABLE: ///���ø澯����
		LocationInfor.AlarmEnable = ZRev[1] << 0;
		memcpy1(HeartBuf,ZRev, strlen((char *)ZRev));
		
		///����flash
		FlashWrite16(MOVE_ENABLE_ADDR,(uint16_t *)&LocationInfor.AlarmEnable,1);	
	
		break;
		
		case MOVE_CHECK_MOVE_ENABLE:
		HeartBuf[1] = LocationInfor.AlarmEnable;
	
		break;
		
		case QUERY_CHECK_VER:
		HeartBuf[1] = LocationInfor.Versions;	
		break;
		
		case QUERY_DEV_LOCA:
			///������������: QUERY_FEED_BACK
		
		break;
		
		case QUERY_OSC_STATE: ///���ٶȴ�������ֵ
		///HeartBuf[1] = �񶯿���ֵ(ʵ��ָ��?);	
			
		break;
		
		case QUERY_DEV_INFOR:
//		HeartBuf[1] = ��ʼ�񶯿�����ֵ
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

/*GetLocation����λ�����������
*����				 : GpsLocation ��LocationCmd����������
*����				 �������������ṩzetaӦ��
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
	NorthLat, &Nstate, EastLot, &Estate);     ////ȡ����,��ֹ��ͬʱ����,

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
					
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 20)&0xFF; ///28bitȡ��8bit��ע���32bitȡ����
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 12)&0xFF;
	GpsBuf[len++] = (SetGpsMode.EastSpend >> 4)&0xFF;
	
	GpsBuf[len++] = (((SetGpsMode.EastSpend >> 0) & 0xF) << 4) | ((SetGpsMode.NorthSpend >> 24) & 0xF); ///28bitȡ��4bit
	
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 16) & 0xFF;
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 8) & 0xFF;
	GpsBuf[len++] = (SetGpsMode.NorthSpend >> 0) & 0xFF;	
	
	DEBUG_APP(2,"E_Data = %.4f N_Data = %.4f %d %d\r\n",data_N, data_E, SetGpsMode.NorthSpend, SetGpsMode.EastSpend);
	
	return GpsBuf;
}

/*LocationCheck����ʱɨ��GPS��λ��Ϣ
*����				 	 : Location_t����ʱʱ��
*����				   ����
*/
void LocationCheckGps(LocationIn_t Locat)
{
	if(SetGpsMode.Gpll)
	{
		if(((HAL_GetTick( ) - SetGpsMode.GpsOverTime) > Locat.GpsTime * MSEC) && (LocatHandles->BreakState(  ) == PATIONNULL))  ///GPS 2�����ڶ�λʧ�ܣ�Ĭ��GPS�쳣���ٶ�λ
	 {	 
			DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsMode.GpsOverTime);
			 
			LocatHandles->SetState( PATIONFAIL );
		
			Gps.Disable(  ); ///�ر�GPS
			 
			SetGpsMode.Start = false;
		  SetGpsMode.Gpll = false;

			memset(LocatHandles->Buf, 0, strlen(LocatHandles->Buf));
		 
			gpsx.gpssta = 0;	
						 
//			HAL_TIM_Base_Stop_IT(&htim2);
		}
	}
}

/*LocationSetState������GPS��λ״̬
*����				 	 		: State
*����				   		����
*/
void LocationSetState(uint8_t State)
{
	SetGpsMode.LocationState = State;
}

/*LocationBreakState����ȡGPS��λ״̬
*����				 	 			: ��
*����				   			��State
*/
uint8_t LocationBreakState( void )
{
	return SetGpsMode.LocationState;
}

/*LocationSetMode	�����ö�λ������ģʽ
*����				 	 		: ��λ������ģʽ
*����				   		����
*/
void LocationSetMode( Locatmode_t Mode )
{
	LocationInfor.Mode = Mode;
}

/*LocationGetMode		����ȡ��λ������ģʽ
*����				 	 			: ��
*����				   			��State
*/
Locatmode_t LocationGetMode( void )
{
	return LocationInfor.Mode;
}

/*
*memcpy1��	���ݿ���
*dst��			����Ŀ��
*src:				����Դ��ַ
*size:			���ݴ�С
*���أ�			��
*/
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
	while( size-- )
	{
		*dst++ = *src++;
	}
}

/*
**************************************************************************************************************
*	@file		Location.c
*	@author  Jason
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

LocationIn_t LocationInfor = {false, false, false, false, false, VERSIOS, 12*HOUR, 5, 0x68, 3, 2*MINUTE, 5, 5, 1, HeartMode, InvalidActive, 0, 0}; //30-2*MINUTE 12*HOUR

LocatH_t 	LocatHandle;

LocatH_t 	*LocatHandles;

/*LocationInit����λ����ʼ��
*����					����
*����					����
*/
void LocationInit( void )
{
	LocatHandle.Cmd			 = LocationCmd;
	LocatHandle.GetLoca		 = GetLocation;
	LocatHandle.CheckGps 	 = LocationCheckGps;
	LocatHandle.SetState	 	 = LocationSetState;
	LocatHandle.BreakState   = LocationBreakState;
	LocatHandle.SetMode		 = LocationSetMode;
	LocatHandle.GetMode		 = LocationGetMode;
	
	LocatHandles = &LocatHandle;
	
	LocatHandles->SetState(PATIONNULL);
	
	LocatHandles->SetMode(WaitMode);
	
	LocationInfor.MotionState = InvalidActive;
}

/*LocationCmd ����λ�����������
*����		 	  ��Zeta_t
*����		 	  �������������ṩzetaӦ��
*/
uint8_t *LocationCmd(Zeta_t *ZRev)
{
	uint8_t	*HeartBuf = ZRev->RevBuf;
	
	uint16_t Sqrtdata = 0;
	
	ZetaSendBuf.Len = 0;
		
	HeartBuf[ZetaSendBuf.Len++] = ZRev->RevBuf[0];
	
	switch(ZRev->RevBuf[0])
	{
		case HEART_SET_CYCLE: ///������������		
			LocationInfor.HeartCycle  = (ZRev->RevBuf[ZetaSendBuf.Len++] << 8);
			LocationInfor.HeartCycle |= ZRev->RevBuf[ZetaSendBuf.Len++];
			
			DEBUG_APP(2,"HeartCycle = %04X %02X %02X %02X\r\n",LocationInfor.HeartCycle, HeartBuf[0],HeartBuf[1],HeartBuf[2]);
			///����flash
			FlashWrite16(HEART_CYCLE_ADDR,&LocationInfor.HeartCycle,1);	
			
		break;
		
		case HEART_CHECK_CYCLE: ///��ѯ����
			LocationInfor.HeartCycle 	 = FlashRead16(HEART_CYCLE_ADDR);;			
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 0)&0xff;

			DEBUG_APP(2,"HeartCycle = %04X %02X %02X\r\n",LocationInfor.HeartCycle, HeartBuf[1],HeartBuf[2]);
						
		break;
		
		case ALARM_SET_CYCLE:	///���ø澯����
			LocationInfor.AlarmCycle  = ZRev->RevBuf[ZetaSendBuf.Len++];
			HeartBuf[ZetaSendBuf.Len] = LocationInfor.AlarmCycle;
		
			uint16_t AlarmCycle = 0;
		
			if(LocationInfor.AlarmCycle == 0)
			{
				AlarmCycle = 0x55aa;
			}
			else
			{
				AlarmCycle = LocationInfor.AlarmCycle;
			}
			
			///����flash
			FlashWrite16(ALARM_CYCLE_ADDR, &AlarmCycle,1);	
		
		break;
		
		case ALARM_CHECK_CYCLE: ///��ѯ�澯
			
			if(FlashRead16(ALARM_CYCLE_ADDR) == 0x55aa)
			{
				LocationInfor.AlarmCycle = 0;
			}
			else
			{			
				LocationInfor.AlarmCycle = FlashRead16(ALARM_CYCLE_ADDR);
			}				

			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.AlarmCycle >> 0)&0xff;
		
		break;
		
		case GPS_SET_LOCA_TIME: ///��λ��ʱʱ��
			LocationInfor.GpsTime = ZRev->RevBuf[ZetaSendBuf.Len++] << 8;
			LocationInfor.GpsTime |= ZRev->RevBuf[ZetaSendBuf.Len++] << 0;	
			
			DEBUG_APP(2,"LocationInfor.GpsTime = %04X %02X %02X\r\n",LocationInfor.GpsTime, HeartBuf[1],HeartBuf[2]);
			
			///����flash
			FlashWrite16(GPS_LOCA_TIME_ADDR,&LocationInfor.GpsTime,1);	
			
		break;
		
		case GPS_CHECK_LOCA_TIME: 
			LocationInfor.GpsTime = FlashRead16(GPS_LOCA_TIME_ADDR);
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.GpsTime >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.GpsTime >> 0)&0xff;
			
			DEBUG_APP(2,"LocationInfor.GpsTime = %04X %02X %02X\r\n",LocationInfor.GpsTime, HeartBuf[1],HeartBuf[2]);
			
		break;
		
		case MOVE_SET_MOVE_CONDITION: ///�����ƶ��ж�ʱ��
			LocationInfor.MoveTimes 	= ZRev->RevBuf[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.MoveTimes;
			
			///����flash
			FlashWrite16(MOVE_CONDITION_ADDR,(uint16_t *)&LocationInfor.MoveTimes,1);	
		
			DEBUG_APP(2,"LocationInfor.MoveTimes = %d\r\n",LocationInfor.MoveTimes);
			
		break;
		
		case MOVE_CHECK_MOVE_CONDITION:
			LocationInfor.MoveTimes 	 = FlashRead16(MOVE_CONDITION_ADDR);

			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.MoveTimes;
		
			DEBUG_APP(2,"LocationInfor.MoveTimes = %d\r\n",LocationInfor.MoveTimes);
		
		break;
		
		case MOVE_SET_STOP_CONDITION: ///����ֹͣ�ƶ��ж�ʱ��
			LocationInfor.StopTimes 	= ZRev->RevBuf[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.StopTimes;
		
			///����flash
			FlashWrite16(MOVE_STOP_CONDITION_ADDR,(uint16_t *)&LocationInfor.StopTimes,1);
		
			DEBUG_APP(2,"LocationInfor.StopTimes = %d\r\n",LocationInfor.StopTimes);
	
		break;
		
		case MOVE_CHECK_STOP_CONDITION:
			LocationInfor.StopTimes		 = FlashRead16(MOVE_STOP_CONDITION_ADDR);		
			
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.StopTimes;	
		
			DEBUG_APP(2,"LocationInfor.StopTimes = %d\r\n",LocationInfor.StopTimes);
		
		break;
		
		case MOVE_SET_MOVE_ENABLE: ///���ø澯����
			LocationInfor.AlarmEnable 	= ZRev->RevBuf[ZetaSendBuf.Len++] << 0;
			HeartBuf[ZetaSendBuf.Len] 	= LocationInfor.AlarmEnable;
			
			///����flash
			FlashWrite16(MOVE_ENABLE_ADDR,(uint16_t *)&LocationInfor.AlarmEnable,1);				
	
		break;
		
		case MOVE_CHECK_MOVE_ENABLE:
			LocationInfor.AlarmEnable 	 = FlashRead16(MOVE_ENABLE_ADDR);		

			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmEnable;
	
		break;
		
		case QUERY_CHECK_VER:
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.Versions;	
		break;
		
		case SET_MMA8452_PARAM:
			
			switch(ZRev->RevBuf[1])
			{
				case 0x01: //50hz				
					LocationInfor.Mma8452DaRte = ASLP_RATE_20MS | DATA_RATE_20MS;
					break;
				
				case 0x02: //12.5hz
					LocationInfor.Mma8452DaRte = ASLP_RATE_80MS | DATA_RATE_80MS;			
					break;
				
				case 0x03: ///6.25hz
					LocationInfor.Mma8452DaRte = ASLP_RATE_160MS | DATA_RATE_160MS;
					break;
				
				case 0x04: ///1.56hz
					LocationInfor.Mma8452DaRte = ASLP_RATE_640MS | DATA_RATE_640MS;
					break;
				
				default:					
					break;
			}
			
			if(ZRev->RevBuf[2] == 0x00)
			{
				ZRev->Ack	= false;
				break;	
			}
			
			LocationInfor.Mma8452MCount = ZRev->RevBuf[2];

			///����flash
			FlashWrite16(MMA8452_DATA_RATE,(uint16_t *)&LocationInfor.Mma8452DaRte,1);
			FlashWrite16(MMA845_MT_COUNT,  (uint16_t *)&LocationInfor.Mma8452MCount,1);
			
			MMA845xInit(LocationInfor);
			
			HeartBuf[ZetaSendBuf.Len++] = ZRev->RevBuf[1];	
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.Mma8452MCount;
		break;
		
		case GET_MMA8452_PARAM:
			
			LocationInfor.Mma8452DaRte  = FlashRead16(MMA8452_DATA_RATE);
			LocationInfor.Mma8452MCount = FlashRead16(MMA845_MT_COUNT);
		
			switch(LocationInfor.Mma8452DaRte)
			{
				case ASLP_RATE_20MS | DATA_RATE_20MS:
					HeartBuf[ZetaSendBuf.Len++] = 0x01;
				break;
				
				case ASLP_RATE_80MS | DATA_RATE_80MS:
					HeartBuf[ZetaSendBuf.Len++] = 0x02;
				break;
				
				case ASLP_RATE_160MS | DATA_RATE_160MS:
					HeartBuf[ZetaSendBuf.Len++] = 0x03;
				break;
				
				case ASLP_RATE_640MS | DATA_RATE_640MS:
					HeartBuf[ZetaSendBuf.Len++] = 0x04;
				break;
				
				default:
					break;				
			}
				
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.Mma8452MCount;	
			
		break;
		
		case QUERY_DEV_LOCA:
		///������������: QUERY_FEED_BACK	
			LocatHandles->SetMode( QueryLocaMode );
		
			DEBUG_APP(2,"QUERY_DEV_LOCA = %d",QUERY_DEV_LOCA);

		break;
				
		case QUERY_DEV_INFOR:
			Sqrtdata = MMA8452MultipleRead(  );
		
			HeartBuf[ZetaSendBuf.Len++] = (Sqrtdata >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (Sqrtdata >> 0)&0xff;
		
			LocationInfor.HeartCycle 	 = FlashRead16(HEART_CYCLE_ADDR);

			LocationInfor.AlarmCycle 	 = FlashRead16(ALARM_CYCLE_ADDR);
			
			LocationInfor.AlarmEnable 	 = FlashRead16(MOVE_ENABLE_ADDR);		
			
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 8)&0xff;
			HeartBuf[ZetaSendBuf.Len++] = (LocationInfor.HeartCycle >> 0)&0xff;
			
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmCycle;
			HeartBuf[ZetaSendBuf.Len++] = LocationInfor.AlarmEnable;
		
		break;
		
		default:
			ZRev->Ack	= false; 
		
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
	char		GPLL[10] = {0};
	char 		NorthLat[15] = {0};
	char 		EastLot[15] = {0};

	char 		Nstate = 0;
	char 		Estate = 0;

	double 	data_N = 0;
	double 	data_E = 0;
	
	uint8_t len = 0;
	
	uint8_t BufTemp[8] = {0};
	
	uint8_t *GpsBuf = BufTemp; ///����ַ��ֱ�Ӵ��ݻ������

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
	
	DEBUG_APP(3,"LocationCmd = %02x %02x %02x %02x",LocationCmd, (LocationCmd << 2), (SetGpsMode.West << 1),(SetGpsMode.South << 0));
	
	BufTemp[len++] = (LocationCmd << 2) | (SetGpsMode.West << 1) | (SetGpsMode.South << 0);
						
	BufTemp[len++] = (SetGpsMode.EastSpend >> 20)&0xFF; ///28bitȡ��8bit��ע���32bitȡ����
	BufTemp[len++] = (SetGpsMode.EastSpend >> 12)&0xFF;
	BufTemp[len++] = (SetGpsMode.EastSpend >> 4)&0xFF;
	
	BufTemp[len++] = (((SetGpsMode.EastSpend >> 0) & 0xF) << 4) | ((SetGpsMode.NorthSpend >> 24) & 0xF); ///28bitȡ��4bit
	
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 16) & 0xFF;
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 8) & 0xFF;
	BufTemp[len++] = (SetGpsMode.NorthSpend >> 0) & 0xFF;	
	
	DEBUG_APP(2,"E_Data = %.4f N_Data = %.4f %d %d\r\n",data_N, data_E, SetGpsMode.NorthSpend, SetGpsMode.EastSpend);
	
	return GpsBuf;
}

/*LocationCheck����ʱɨ��GPS��λ��Ϣ
*����				: Location_t����ʱʱ��
*����				����
*/
void LocationCheckGps(LocationIn_t Locat)
{
	if(SetGpsMode.Gpll)
	{	
		if( ((HAL_GetTick( ) - SetGpsMode.GpsOverTime) > Locat.GpsTime * MSEC) )  ///GPS 2�����ڶ�λʧ�ܣ�Ĭ��GPS�쳣���ٶ�λ && (LocatHandles->BreakState(  ) == PATIONNULL)
		{	 
			DEBUG(2,"GPS_TIME22 : %d\r\n",HAL_GetTick( ) - SetGpsMode.GpsOverTime);

			LocatHandles->SetState( PATIONFAIL );

			Gps.Disable(  ); ///�ر�GPS

			SetGpsMode.Start = false;
			SetGpsMode.Gpll = false;

			memset(LocatHandles->Buf, 0, strlen(LocatHandles->Buf));

			gpsx.gpssta = 0;	
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
*dst��		����Ŀ��
*src:			����Դ��ַ
*size:		���ݴ�С
*���أ�		��
*/
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
	while( size-- )
	{
		*dst++ = *src++;
	}
}

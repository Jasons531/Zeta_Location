/*
**************************************************************************************************************
*	@file		Location.h
*	@author 	Jason
*	@version 
*	@date    
*	@brief	    
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOCATION_H
#define __LOCATION_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32l0xx_hal.h"
#include "zeta.h"

/**********************�������start*********************//*
************�����ϱ�************/
#define 	HEART_REPORT									0x00

/************�澯�ϱ���λ�ɹ�************/
#define 	ALARM_REP_LOCA_SUCESS					0x01

/************�澯�ϱ���λʧ��************/
#define 	ALARM_REP_LOCA_FAIL						0x02

/************�ʲ���ֹ����λ�ɹ�************/
#define		MOVE_STATIC_LOCA_SUCESS				0x03
	
/************�ʲ���ֹ����λʧ��************/
#define		MOVE_STATIC_LOCA_FAIL					0x04

/************��ѯ����************/
#define		QUERY_FEED_BACK								0X05

/************���Ͱ汾ID���ϵ��һ�η���************/
#define		QUERY_SEND_VER								0X80	

/**********************�������end*********************/

/**********************�������start*********************//*
************������������************/
#define 	HEART_SET_CYCLE								0x40

/************��ѯ��������************/
#define 	HEART_CHECK_CYCLE							0x41

/************���ø澯����************/
#define		ALARM_SET_CYCLE								0x42
	
/************��ѯ�澯����************/
#define		ALARM_CHECK_CYCLE							0x43

/************���ö�λʱ��************/
#define		GPS_SET_LOCA_TIME							0x44
	
/************��ѯ��λʱ��************/
#define		GPS_CHECK_LOCA_TIME						0x45
	
/************�����ƶ�����************/
#define		MOVE_SET_MOVE_CONDITION				0x50
	
/************��ѯ�ƶ�����************/
#define		MOVE_CHECK_MOVE_CONDITION			0X51
	
/************�����ʲ��ƶ�ֹͣ����************/
#define		MOVE_SET_STOP_CONDITION				0X60
	
/************��ѯ�ʲ��ƶ�ֹͣ����************/
#define		MOVE_CHECK_STOP_CONDITION			0X61
	
/************�����ʲ��ƶ��澯����************/
#define		MOVE_SET_MOVE_SWITCH					0X70

/************��ѯ�ʲ��ƶ��澯����************/
#define		MOVE_CHECK_MOVE_SWITCH				0X71
		
/************��ѯ�汾ID************/
#define		QUERY_CHECK_VER								0X81

/************��ѯ�豸λ��************/
#define		QUERY_DEV_LOCA								0XE0
	
/************��ѯ�񶯿���״̬************/
#define		QUERY_OSC_STATE								0XE1

/************��ѯ�豸��Ϣ************/
#define		QUERY_DEV_INFOR								0XE2
/**********************�������end*********************/

typedef struct LocationI_s
{
	//����
	uint16_t 	HeartCycle;
	
	/************�澯���ڣ�0��ֻ�ϱ�һ��/����************/
	uint8_t 	AlarmCycle;	
	
		/************GPS��λ״̬************/
	uint8_t  	GpsStates;
	
	/************GPS��λ��ʱʱ��/��************/
	uint16_t  GpsTime;
	
	/************�ʲ��ƶ�����/��************/
	uint8_t	  MoveTimes;
	
	/************�ʲ�ֹͣ����/��************/
	uint8_t	  StopTimes;
	
	/************�ʲ��ƶ����أ�0�����澯��1���澯************/
	uint8_t   AlarmSwitch;
	
	uint8_t 	Versions;

}LocationIn_t;


typedef struct LocatH_s
{
 uint8_t 	*(*Cmd)(uint8_t *ZRev);

 uint8_t 	*(*GetLoca)(char *GpsLocation, uint8_t LocationCmd);

 void 		(*CheckGps)(LocationIn_t Locat);

 void 		(*SetState)(uint8_t State);

 uint8_t 	(*BreakState)( void ); 
}LocatH_t;

extern LocationIn_t LocationInfor;

extern LocatH_t LocatHandle;

extern LocatH_t *LocatHandles;

void LocationInit(void);

extern uint8_t *LocationCmd(uint8_t *ZRev);

extern uint8_t *GetLocation(char *GpsLocation, uint8_t LocationCmd);

extern void LocationCheckGps(LocationIn_t Locat);

extern void LocationSetState(uint8_t State);

extern uint8_t LocationBreakState( void );

void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size );



#endif /* __LOCATION_H  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
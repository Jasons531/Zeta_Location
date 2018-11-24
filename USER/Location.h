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

/**********************上行命令：start*********************//*
************心跳上报，定位成功************/
#define 	HEART_REPORT_SUCESS						0x00

/************心跳上报，定位失败************/
#define 	HEART_REPORT_FAIL							0x0F

/************告警上报定位成功************/
#define 	ALARM_REP_LOCA_SUCESS					0x01

/************告警上报定位失败************/
#define 	ALARM_REP_LOCA_FAIL						0x02

/************资产静止，定位成功************/
#define		MOVE_STATIC_LOCA_SUCESS				0x03
	
/************资产静止，定位失败************/
#define		MOVE_STATIC_LOCA_FAIL					0x04

/************查询反馈，定位成功************/
#define		QUERY_FEED_LOCA_SUCESS				0X05

/************查询反馈，定位失败************/
#define		QUERY_FEED_LOCA_FAIL					0X06

/************发送版本ID：上电第一次发送************/
#define		QUERY_SEND_VER								0X80	

/**********************上行命令：end*********************/

/**********************下行命令：start*********************//*
************设置心跳周期 ：分钟************/
#define 	HEART_SET_CYCLE								0x40

/************查询心跳周期 ：分钟************/
#define 	HEART_CHECK_CYCLE							0x41

/************设置告警周期************/
#define		ALARM_SET_CYCLE								0x42
	
/************查询告警周期************/
#define		ALARM_CHECK_CYCLE							0x43

/************设置定位时间************/
#define		GPS_SET_LOCA_TIME							0x44
	
/************查询定位时间************/
#define		GPS_CHECK_LOCA_TIME						0x45
	
/************设置移动条件************/
#define		MOVE_SET_MOVE_CONDITION				0x50
	
/************查询移动条件************/
#define		MOVE_CHECK_MOVE_CONDITION			0X51
	
/************设置资产移动停止条件************/
#define		MOVE_SET_STOP_CONDITION				0X60
	
/************查询资产移动停止条件************/
#define		MOVE_CHECK_STOP_CONDITION			0X61
	
/************设置资产移动告警开关************/
#define		MOVE_SET_MOVE_ENABLE					0X70

/************查询资产移动告警开关************/
#define		MOVE_CHECK_MOVE_ENABLE				0X71
		
/************查询版本ID************/
#define		QUERY_CHECK_VER								0X81

/************查询设备位置************/
#define		QUERY_DEV_LOCA								0XE0
	
/************查询振动开关状态************/
#define		QUERY_OSC_STATE								0XE1

/************查询设备信息************/
#define		QUERY_DEV_INFOR								0XE2
/**********************下行命令：end*********************/

typedef struct LocationI_s
{
	//分钟
	uint16_t 	HeartCycle;
	
	/************告警周期：0则只上报一次/分钟************/
	uint8_t 	AlarmCycle;	
	
		/************GPS定位状态************/
	uint8_t  	GpsStates;
	
	/************GPS定位超时时间/秒************/
	uint16_t  GpsTime;
	
	/************资产移动条件/秒************/
	uint8_t	  MoveTimes;
	
	/************资产停止条件/秒************/
	uint8_t	  StopTimes;
	
	/************资产移动开关：0：不告警，1：告警************/
	uint8_t   AlarmEnable;
	
	uint8_t 	Versions;

}LocationIn_t;


typedef struct LocatH_s
{
 char 		Buf[54];
	
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

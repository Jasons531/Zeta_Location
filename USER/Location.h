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
#define		QUERY_FEED_LOCA_SUCESS				0x05

/************查询反馈，定位失败************/
#define		QUERY_FEED_LOCA_FAIL					0x06

/************发送版本ID：上电第一次发送************/
#define		QUERY_SEND_VER								0x80	

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
#define		MOVE_CHECK_MOVE_CONDITION			0x51
	
/************设置资产移动停止条件************/
#define		MOVE_SET_STOP_CONDITION				0x60
	
/************查询资产移动停止条件************/
#define		MOVE_CHECK_STOP_CONDITION			0x61
	
/************设置资产移动告警开关************/
#define		MOVE_SET_MOVE_ENABLE					0x70

/************查询资产移动告警开关************/
#define		MOVE_CHECK_MOVE_ENABLE				0x71
		
/************查询版本ID************/
#define		QUERY_CHECK_VER								0x81

/************查询设备位置************/
#define		QUERY_DEV_LOCA								0xE0
	
/************查询振动开关状态************/
#define		QUERY_OSC_STATE								0xE1

/************查询设备信息************/
#define		QUERY_DEV_INFOR								0xE2
/**********************下行命令：end*********************/

typedef enum Locatmode_s
{
	/*********等待模式********/
	WaitMode				= 0,
	
	/*********心跳模式********/
	HeartMode				= 1,
	
	/*********运动模式********/
	MotionMode			= 2,
	
	/*********运动停止模式********/
	MotionStopMode	= 3,
}Locatmode_t;

typedef enum Motion_s
{
	Invalid					= 0,
	Active 					= 1,
	MultActive 			= 2,
	InActive				= 3,	
}Motion_t;

typedef struct LocationI_s
{
	/************开始定位************/
	bool 					BegainLocat;
	
	uint8_t 			Versions;

	//分钟
	uint16_t 			HeartCycle;
	
	/************告警周期：0则只上报一次/分钟************/
	uint8_t 			AlarmCycle;	
	
		/************GPS定位状态************/
	uint8_t  			GpsStates;
	
	/************GPS定位超时时间/秒************/
	uint16_t  		GpsTime;
	
	/************资产移动条件/秒************/
	uint8_t	  		MoveTimes;
	
	/************资产停止条件/秒************/
	uint8_t	  		StopTimes;
	
	/************资产移动开关：0：不告警，1：告警************/
	uint8_t   		AlarmEnable;
	
	/********************定位器工作模式******************/
	Locatmode_t 	Mode;
	
	/************移动状态************/
	Motion_t 			MotionState;

}LocationIn_t;

typedef struct LocatH_s
{
	char 					Buf[54];
	
	uint8_t 			*(*Cmd)( uint8_t *ZRev );

	uint8_t	 			*(*GetLoca)( char *GpsLocation, uint8_t LocationCmd );

	void 					(*CheckGps)( LocationIn_t Locat );

	void 					(*SetState)( uint8_t State );

	uint8_t 			(*BreakState)( void ); 

	void 					(*SetMode)(	Locatmode_t Mode );
	
	Locatmode_t		(*GetMode)( void );
	
}LocatH_t;

extern 	LocationIn_t 	LocationInfor;

extern 	LocatH_t 			LocatHandle;

extern 	LocatH_t 			*LocatHandles;

extern	void 					LocationInit( void );

extern 	uint8_t 			*LocationCmd( uint8_t *ZRev );

extern 	uint8_t				*GetLocation( char *GpsLocation, uint8_t LocationCmd );

extern 	void					LocationCheckGps( LocationIn_t Locat );

extern 	void 					LocationSetState( uint8_t State );

extern 	uint8_t 			LocationBreakState( void );

extern 	void 					LocationSetMode( Locatmode_t Mode );

extern 	Locatmode_t 	LocationGetMode( void );

extern	void 					memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size );



#endif /* __LOCATION_H  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

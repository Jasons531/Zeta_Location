/*
**************************************************************************************************************
*	@file	user-app.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>

#include "Location.h"
#include "board.h"
#include "Zeta.h"

#define GPSEXIST					0x31

#define ACKCOM						0xFE

#define ACKMAC						0xFC

typedef enum cmd_s
{
	MAC 						= 0,
	NETIME					= 1,
	COUNTER 				= 2,
	RSSI 						= 3,
}cmd_t;

typedef struct UserZeta_s
{
	/**************Zeta通讯命令************/
	uint8_t 				Cmd;
	
	/**************Zeta通讯超时************/
	uint32_t				Timeout;
	
	/**************Zeta通讯回复状态************/
	ZetaState_t 		Expect_retval;
}UserZeta_t;

typedef struct User_s
{
	uint8_t 				BatState;
	uint8_t 				CurrentDate;
	uint8_t 				AlarmDate;
	uint32_t 				SleepTime;
	uint32_t        SaveSleepTime;
	uint32_t				TimerCounter;
	bool 						SleepWakeUp;
	bool 						TestMode;
}User_t;

extern 	UserZeta_t UserZetaCheck[];

extern 	uint8_t DeviceInfo[4];

extern 	User_t User;

extern  bool setheart;

void 	 	UserKeyPinInit(void);

void	 	UserKeyWakeupHandle(void);

void	 	UserPeriPheralInit(void);

void	 	UserWakeupHandle(void);

void	 	UserCheckGps(void);

void	 	UserLocationVerion(uint8_t VerCmd);

void	 	UserSendLocation(uint8_t LocationCmd);

void	 	UserLocatMotion(void);

void	 	UserLocatMotionStop(void);

void 		UserLocatDetect(void);

void	 	UserLocatReport(void);

void	 	UserSend(Zeta_t *SendBuf);

void	 	UserSendGps(LocationIn_t LocatCmd);

void	 	UserSendTest(void);

void	 	UserDownCommand(void);

void	 	UserCheckCmd(UserZeta_t *UserZetaCheckCmd);

void	 	UserSetHeart(uint8_t mode);

void	 	UserSetTimer(ZetaTimer_t Timer);

void	 	UserCloseTimer(ZetaTimer_t Timer);

void	 	UserIntoLowPower(void);

void	 	String_Conversion(char *str, uint8_t *src, uint8_t len);

void	 	UserReadFlash(void);

#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

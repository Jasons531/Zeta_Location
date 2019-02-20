/*
**************************************************************************************************************
*	@file		Zeta.h
*	@author Jason
*	@version 
*	@date    
*	@brief	Zeta串口处理
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ZETA_H
#define __ZETA_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32l0xx_hal.h"

#define  ZETAINT_IO						GPIOB
#define  ZETAINT_PIN   				GPIO_PIN_1

#define  ZETAWAKUP_IO					GPIOB
#define  ZETAWAKUP_PIN				GPIO_PIN_0

#define  ZETASTATU_IO					GPIOB
#define  ZETASTATU_PIN				GPIO_PIN_2

#define  ZETAPOWER_IO					GPIOA
#define  ZETAPOWER_PIN				GPIO_PIN_11

#define  ZETAMAXLEN						38

#define  FIXLEN								11

/*
*ZetaState_t: Zeta状态列表
*/

typedef enum uZetaState
{
	/***********数据帧应答***********/
	DataAck 				= 0x01,
	
	/***********缓存区满***********/
	BufFull 				= 0x02,
	
	/***********长度出错***********/
	LenError 				= 0x03,
	
	/***********设置成功***********/
	SetSecess 			= 0x20,
	
	/***********设置失败***********/
	SetFail 				= 0x21,
	
	/***********无效警告***********/
	InvaildWarning 	= 0x40,
	
	/***********关停提示***********/
	StopPrompt			= 0x41,
	
	/***********剩发数为0***********/
	SendDatafalse 	= 0x42,
	
	/***********终端未注册***********/
	Unregistered		= 0x43,
	
	/***********数据域有效：获取参数/接收数据***********/
	Payload					= 0x44,
	
	/***********默认初始状态***********/
	Reset						= 0x45
	
}ZetaState_t;

/*
*Zeta_t: Zeta发送、接收处理缓存
*/
typedef struct uZeta
{
	uint8_t 		Len;
	uint8_t			MaxLen;
	uint8_t			Buf[50];
	uint8_t			RevBuf[10];
	uint32_t    Uart_time;
	ZetaState_t States;	
	bool				Ack;
}Zeta_t;

/*
*ZetaTimer_t: Zeta设置定时器参数
*/
typedef struct uZetaTimer
{
	uint8_t 		*StartTime;
	uint8_t			*SpaceTime; ///间隔时间
	uint8_t    	TimerID;
}ZetaTimer_t;

/*
*ZetaHandle_t: Zeta处理功能代码
*/
typedef struct uZetaHandle
{
	void 					(*Init)(void);
	void 					(*PowerOn)(void);
	void 					(*PowerOff)(void);
	void 					(*WakeupEnable)(void);
	void 					(*WakeupDisable)(void);
	void 					(*Interrupt)(void);
	void 					(*Send)(Zeta_t *ZetaBuf);
	ZetaState_t		(*Recv)(void);
	uint8_t 			(*CRC8)(uint8_t *ptr, uint8_t len);
	GPIO_PinState (*Status)(void);
	uint8_t	      (*DownCommand)(uint8_t *RevBuf);
}ZetaHandle_t;

extern Zeta_t 			ZetaRecviceBuf;

extern Zeta_t 			ZetaSendBuf;

extern ZetaTimer_t	ZetaTimer;

extern const ZetaHandle_t ZetaHandle;

void ZetaInit(void);

void ZetaPowerOn(void);

void ZetaPowerOff(void);

void WakeupZetaEnable(void);

void WakeupZetaDisable(void);

void ZetaInterrupt(void);

GPIO_PinState ZetaStatus(void);

void ZetaSend(Zeta_t *ZetaBuf);

ZetaState_t ZetaRecv(void);

uint8_t ZetaDownCommand(uint8_t *RevBuf);

uint8_t CalcCRC8(uint8_t *ptr, uint8_t len);


#endif /* __ZETA_H  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

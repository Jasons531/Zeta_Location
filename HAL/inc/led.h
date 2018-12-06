/*
**************************************************************************************************************
*	@file	led.h
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief	ÑÓÊ±º¯Êý
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H

#define LED_PORT			GPIOB

#define LED_RED				GPIO_PIN_6

#define LED_GREEN			GPIO_PIN_5


#include <stdint.h>
#include "timer.h"

typedef enum ledStates
{
    NoneCare = 0,
    GpsLocation = 1,
    SendSucess = 2,
    SendFail = 3,
    Receive = 4,
}LedStates_t;

extern LedStates_t 	LedStates;

extern LedStates_t 	LedSaveState;

extern void 				SetLedStates(uint8_t States);

extern LedStates_t 	GetLedStates(void);

extern void 				LedInit(void);

extern void 				PowerOn(void);

extern void 				PowerOff(void);

extern void 				LedOn(void);

extern void 				LedOff(void);

extern void 				LedToggle(void);

extern void 				LedSendSucess(int8_t Counter);

extern void 				LedSendFail(int8_t Counter);

extern void 				LedRev(int8_t Counter);


#endif /* __LED_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

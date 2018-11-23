/*
**************************************************************************************************************
*	@file	gpio.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "board.h"


/********************DEBUG UART******************/
#define  USART1_IO									GPIOA
#define  USART1_TX									GPIO_PIN_9
#define  USART1_RX									GPIO_PIN_10

/********************GPS UART******************/
#define  USART2_IO									GPIOA
#define  USART2_TX									GPIO_PIN_2
#define  USART2_RX									GPIO_PIN_3

/********************GPS Power******************/
#define  GPS_IO											GPIOB
#define  GPS_Power_ON     					GPIO_PIN_12

/********************I2C******************/
#define	 I2C2_IO										GPIOB
#define  I2C2_SCL										GPIO_PIN_13
#define	 I2C2_SDA  									GPIO_PIN_14

/********************LED******************/
#define  LED_IO											GPIOB
#define  LED_MCU										GPIO_PIN_7
#define  LED_RED										GPIO_PIN_6
#define  LED_GREEN									GPIO_PIN_5

/**********************MMA8452INT_1_IO*******************/
#define  MMA8452INT_1_IO						GPIOA
#define  MMA8452INT_1								GPIO_PIN_8

/**********************MMA8452INT_2_IO*******************/
#define  MMA8452INT_2_IO						GPIOB
#define  MMA8452INT_2								GPIO_PIN_15

/**********************MMA8452INT_WAKE_IO*******************/
#define  MMA8452INT_WAKE_IO					GPIOC
#define  MMA8452INT_WAKE						GPIO_PIN_13

/**********************POWER_SWTICH*******************/
#define  POWER_KEY_IO								GPIOA
#define  POWER_KEY									GPIO_PIN_0

#define KEY_ON			   							1
#define KEY_OFF			   							0


#endif // __GPIO_H__

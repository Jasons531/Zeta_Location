/*
**************************************************************************************************************
*	@file	timer-board.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief	Description: MCU timer
***************************************************************************************************************
*/


#ifndef __TIMER_BOARD_H__
#define __TIMER_BOARD_H__

#include <stdint.h>
#include "board.h"


extern TIM_HandleTypeDef htim2;


/*!
 * \brief Initializes the timer
 *
 * \remark The timer is based on TIM2 with a 10uS time basis
 */
void TimerHwInit( void );

/*!
 * \brief DeInitializes the timer
 */
void TimerHwDeInit( void );


#endif // __TIMER_BOARD_H__

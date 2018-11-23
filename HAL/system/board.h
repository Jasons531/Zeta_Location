/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_tim.h"
#include "led.h"
#include "gpio.h"
#include "gpio-board.h"
#include "i2c.h"
#include "rtc-board.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "user-app.h"
#include "stmflash.h"
#include "power.h"
#include "debug.h"
#include "spi.h"
#include "timer-board.h"
#include "Zeta.h"
#include "wwdg.h"
#include "mma8452q.h"

/*!
 * Unique Devices IDs register set ( STM32F1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80050+0x14 )  ///U_ID(95:64):

/*!
 * Random seed generated using the MCU Unique ID
 */
#define RAND_SEED                                   ( ( *( uint32_t* )ID1 ) ^ \
                                                      ( *( uint32_t* )ID2 ) ^ \
                                                      ( *( uint32_t* )ID3 ) )


/**************记录系统时间作为休眠时间校准*****************/
extern volatile uint32_t system_time;

extern bool McuInitialized;

enum BoardPowerSource
{
    USB_POWER = 0,
    BATTERY_POWER
};

/*!
 * System Clock Configuration
 */
void SystemClockConfig( void );

/*!
 * \brief Initializes System Clock Configuration.
 */
void BoardInitClock( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

void BoardEnterStandby(void);

void SystemClockReConfig( void );

#endif // __BOARD_H__

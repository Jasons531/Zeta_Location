/***********************************************************************************************\
* Freescale MMA8451,2,3Q Driver
*
* Filename: system.h
*
*
* (c) Copyright 2010, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
\***********************************************************************************************/
#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#define		byte		char

#define 	TRUE		1

#define  	FALSE		0

//#include <hidef.h>                /* for EnableInterrupts macro */
//#include "derivative.h"           /* include peripheral declarations */

/***********************************************************************************************\
* Public type definitions
\***********************************************************************************************/

/***********************************************************************************************
**
**  Variable type definition: BIT_FIELD
*/
typedef union
{
  byte Byte;
  struct {
    byte _0          :1;
    byte _1          :1;
    byte _2          :1;
    byte _3          :1;
    byte _4          :1;
    byte _5          :1;
    byte _6          :1;
    byte _7          :1;
  } Bit;
} BIT_FIELD;


/***********************************************************************************************
**
**  Variable type definition: tword
*/
typedef union
{
  unsigned short Word;
  struct
  {
    byte hi;
    byte lo;
  } Byte;
} tword;


/***********************************************************************************************
**
**  Variable type definition: tfifo_xyz
*/
typedef struct
{
  byte x_msb;
  byte x_lsb;
  byte y_msb;
  byte y_lsb;
  byte z_msb;
  byte z_lsb;
} tfifo_xyz;


/***********************************************************************************************
**
**  Variable type definition: tfifo_sample
*/
typedef union
{
  byte Byte[6];
  struct
  {
    tfifo_xyz XYZ;
  } Sample;
} tfifo_sample;

#define FIFO_BUFFER_SIZE          32


/***********************************************************************************************\
* Project includes
\***********************************************************************************************/

//#include "mma845x.h"              // MMA845xQ macros
//#include "iic.h"                  // IIC macros
//#include "sci.h"                  // SCI macros
//#include "spi.h"                  // SPI macros
//#include "terminal.h"             // Terminal interface macros
//#include "data_flash.h"           // Data Flash macros


#ifdef _MC9S08QE64_H

//Since the boards will all be MC9S08QE8 can eliminate this section.
/*** KBI1SC - KBI1 Status and Control Register; 0x0000000C ***/
#define KBISC                           KBI1SC
#define KBISC_KBIMOD                    KBI1SC_KBIMOD
#define KBISC_KBIE                      KBI1SC_KBIE
#define KBISC_KBACK                     KBI1SC_KBACK
#define KBISC_KBF                       KBI1SC_KBF

#define KBISC_KBIMOD_MASK               KBI1SC_KBIMOD_MASK
#define KBISC_KBIE_MASK                 KBI1SC_KBIE_MASK
#define KBISC_KBACK_MASK                KBI1SC_KBACK_MASK
#define KBISC_KBF_MASK                  KBI1SC_KBF_MASK

/*** KBI1PE - KBI1 Pin Enable Register; 0x0000000D ***/
#define KBIPE                           KBI1PE
#define KBIPE_KBIPE0                    KBI1PE_KBIPE0
#define KBIPE_KBIPE1                    KBI1PE_KBIPE1
#define KBIPE_KBIPE2                    KBI1PE_KBIPE2
#define KBIPE_KBIPE3                    KBI1PE_KBIPE3
#define KBIPE_KBIPE4                    KBI1PE_KBIPE4
#define KBIPE_KBIPE5                    KBI1PE_KBIPE5
#define KBIPE_KBIPE6                    KBI1PE_KBIPE6
#define KBIPE_KBIPE7                    KBI1PE_KBIPE7

#define KBIPE_KBIPE0_MASK               KBI1PE_KBIPE0_MASK
#define KBIPE_KBIPE1_MASK               KBI1PE_KBIPE1_MASK
#define KBIPE_KBIPE2_MASK               KBI1PE_KBIPE2_MASK
#define KBIPE_KBIPE3_MASK               KBI1PE_KBIPE3_MASK
#define KBIPE_KBIPE4_MASK               KBI1PE_KBIPE4_MASK
#define KBIPE_KBIPE5_MASK               KBI1PE_KBIPE5_MASK
#define KBIPE_KBIPE6_MASK               KBI1PE_KBIPE6_MASK
#define KBIPE_KBIPE7_MASK               KBI1PE_KBIPE7_MASK

/*** KBI1ES - KBI1 Edge Select Register; 0x0000000E ***/
#define KBIES                           KBI1ES
#define KBIES_KBEDG0                    KBI1ES_KBEDG0
#define KBIES_KBEDG1                    KBI1ES_KBEDG1
#define KBIES_KBEDG2                    KBI1ES_KBEDG2
#define KBIES_KBEDG3                    KBI1ES_KBEDG3
#define KBIES_KBEDG4                    KBI1ES_KBEDG4
#define KBIES_KBEDG5                    KBI1ES_KBEDG5
#define KBIES_KBEDG6                    KBI1ES_KBEDG6
#define KBIES_KBEDG7                    KBI1ES_KBEDG7

#define KBIES_KBEDG0_MASK               KBI1ES_KBEDG0_MASK
#define KBIES_KBEDG1_MASK               KBI1ES_KBEDG1_MASK
#define KBIES_KBEDG2_MASK               KBI1ES_KBEDG2_MASK
#define KBIES_KBEDG3_MASK               KBI1ES_KBEDG3_MASK
#define KBIES_KBEDG4_MASK               KBI1ES_KBEDG4_MASK
#define KBIES_KBEDG5_MASK               KBI1ES_KBEDG5_MASK
#define KBIES_KBEDG6_MASK               KBI1ES_KBEDG6_MASK
#define KBIES_KBEDG7_MASK               KBI1ES_KBEDG7_MASK

#endif    /* _MC9S08QE64_H */


/***********************************************************************************************\
* Public macros
\***********************************************************************************************/


/***********************************************************************************************
**
**  System control bit flags
*/
#define PROMPT_WAIT                 (SystemFlag.Bit._7)
#define SCI_INPUT_READY             (SystemFlag.Bit._6)
#define INPUT_ERROR                 (SystemFlag.Bit._5)
#define POLL_ACTIVE                 (SystemFlag.Bit._4)
#define ODR_400                     (SystemFlag.Bit._3)


/***********************************************************************************************
**
**  Stream mode control bit flags
*/
#define STREAM_FULLC                  (StreamMode.Bit._7)
#define STREAM_FULLG                  (StreamMode.Bit._6)
#define XYZ_STREAM                    (StreamMode.Bit._5)
#define INT_STREAM                    (StreamMode.Bit._4)
#define OSMode_Normal                 (StreamMode.Bit._3)
#define OSMode_LNLP                   (StreamMode.Bit._2)
#define OSMode_LP                     (StreamMode.Bit._1)
#define OSMode_HiRes                  (StreamMode.Bit._0)
#define STREAM_FULLC_MASK             (0x80)
#define STREAM_FULLG_MASK             (0x40)
#define STREAM_XYZ_MASK               (0x20)
#define STREAM_INT_MASK               (0x10)


/***********************************************************************************************
**
**  Accelerometer functions
*/
enum
{
  FBID_FULL_XYZ_SAMPLE,          //  2 =  XYZ Sample Registers (0x01 - 0x06)
  FBID_8_XYZ_SAMPLE,
  FBID_FIFO,                     // 10 = FIFO
  FBID_MAX
};


/***********************************************************************************************
**
**  General System Control
**
**  0x1802  SOPT1     System Options Register 1
**  0x1803  SOPT2     System Options Register 2
**  0x1808  SPMSC1    System Power Management Status and Control 1 Register
**  0x1809  SPMSC2    System Power Management Status and Control 2 Register
**  0x180B  SPMSC3    System Power Management Status and Control 3 Register
**  0x180E  SCGC1     System Clock Gating Control 1 Register
**  0x180F  SCGC2     System Clock Gating Control 2 Register
**  0x000F  IRQSC     Interrupt Pin Request Status and Control Register
*/

#define init_SOPT1    0b01000010
/*                      1100001U = reset
**                      |||xxx||
**                      |||   |+-- RSTPE      =0  : RESET pin function disabled
**                      |||   +--- BKGDPE     =1  : Background Debug pin enabled
**                      ||+------- STOPE      =0  : Stop Mode disabled
**                      |+-------- COPT       =1  : Long COP timeout period selected
**                      +--------- COPE       =0  : COP Watchdog timer disabled
*/

#define init_SOPT2    0b00000010
/*                      00000000 = reset
**                      |x||x|||
**                      | || ||+-- ACIC1      =0  : ACMP1 output not connected to TPM1CH0 input
**                      | || |+--- IICPS      =1  : SDA on PTB6; SCL on PTB7
**                      | || +---- ACIC2      =0  : ACMP2 output not connected to TPM2CH0 input
**                      | |+------ TPM1CH2PS  =0  : TPM1CH2 on PTA6
**                      | +------- TPM2CH2PS  =0  : TPM2CH2 on PTA7
**                      +--------- COPCLKS    =0  : COP clock source is internal 1kHz reference
*/

#define init_SPMSC1   0b00011100
/*                      00011100 = reset
**                      ||||||x|
**                      |||||| +-- BGBE       =0  : Bandgap buffer disabled
**                      |||||+---- LVDE       =1  : LVD enabled
**                      ||||+----- LVDSE      =1  : LVD enabled during Stop Mode
**                      |||+------ LVDRE      =1  : LVDF=1 causes a reset
**                      ||+------- LVDIE      =0  : LVD interrupt disabled
**                      |+-------- LVDACK
**                      +--------- LVDF
*/

#define init_SPMSC2   0b00000010
/*                      00000010 = reset
**                      |||x||||
**                      ||| |||+-- PPDC       =0  : Stop3 low power mode enabled
**                      ||| ||+--- PPDE       =1  : Partial Power Down enabled
**                      ||| |+---- PPDACK
**                      ||| +----- PPDF
**                      ||+------- LPWUI      =0  : Regulator remains in standby upon interrupt
**                      |+-------- LPRS
**                      +--------- LPR        =0  : Low power run and wait modes disabled
*/

#define init_SPMSC3   0b00000000
/*                      00000000 = reset
**                      ||xx|xxx
**                      ||  +----- LVWIE      =0  : LVW interrupt disabled
**                      |+-------- LVWACK
**                      +--------- LVWF
*/

#define init_SCGC1    0b11111111
/*                      11111111 = reset
**                      x|||x|x|
**                       ||| | +-- SCI        =1  : Bus clock to SCI module enabled
**                       ||| +---- IIC        =1  : Bus clock to IIC module enabled
**                       ||+------ ADC        =1  : Bus clock to ADC module enabled
**                       |+------- TPM1       =1  : Bus clock to TPM1 module enabled
**                       +-------- TPM2       =1  : Bus clock to TPM2 module enabled
*/

#define init_SCGC2    0b11111111
/*                      11111111 = reset
**                      ||||||x|
**                      |||||| +-- SPI        =1  : Bus clock to SPI module enabled
**                      |||||+---- RTC        =1  : Bus clock to RTC module enabled
**                      ||||+----- ACMP       =1  : Bus clock to ACMP module enabled
**                      |||+------ KBI        =1  : Bus clock to KBI module enabled
**                      ||+------- IRQ        =1  : Bus clock to IRQ module enabled
**                      |+-------- FLS        =1  : Bus clock to FLS module enabled
**                      +--------- DBG        =1  : Bus clock to DBG module enabled
*/

#define init_IRQSC    0b00000000
/*                      00000000 = reset
**                      x|||||||
**                       ||||||+-- IRQMOD     =0  : IRQ is edge sensitive only
**                       |||||+--- IRQIE      =0  : IRQF interrupt disabled
**                       ||||+---- IRQACK
**                       |||+----- IRQF
**                       ||+------ IRQPE      =0  : IRQ pin function disabled
**                       |+------- IRQEDG     =0  : IRQ is falling edge / low-level sensitive
**                       +-------- IRQPDD     =0  : IRQ pull device enabled
*/

/***********************************************************************************************
**
**  Internal Clock Source
**
**  0x0038  ICSC1     ICS Control Register 1
**  0x0039  ICSC2     ICS Control Register 2
**  0x003A  ICSTRIM   ICS Trim Register
**  0x003B  ICSSC     ICS Status and Control
**
**  0xFFAE  NVFTRIM
**  0xFFAF  NVICSTRM
**
**  The internal clock reference is trimmed to 36kHz.
**  As per the configuration below, this results in a bus frequency of 9.216MHz.
*/

#define init_ICSC1    0b00000110
/*                      00000100 = reset
**                      ||||||||
**                      |||||||+-- IREFSTEN   =0  : Internal reference clock disabled in Stop
**                      ||||||+--- IRCLKEN    =1  : ICSIRCLK active
**                      |||||+---- IREFS      =1  : Internal reference clock selected
**                      ||+++----- RDIV       =0  : Reference Divide Factor = 1 or 32
**                      ++-------- CLKS       =0  : Output of FLL selected as clock source
*/

#define init_ICSC2    0b00000000
/*                      01000000 = reset
**                      ||||||||
**                      |||||||+-- EREFSTEN   =0  : External reference disabled in Stop
**                      ||||||+--- ERCLKEN    =0  : ICSERCLK inactive
**                      |||||+---- EREFS      =0  : External clock source requested
**                      ||||+----- LP         =0  : FLL is not disabled in bypass mode
**                      |||+------ HGO        =0  : External oscillator configured for low power
**                      ||+------- RANGE      =0  : Low external frequency range selected
**                      ++-------- BDIV       =0  : Bus frequency divider set to divide-by-1
*/

#define init_ICSSC    0b00000000
/*                      0001000U = reset
**                      ||||||||
**                      |||||||+-- FTRIM
**                      ||||||+--- OSCINT
**                      ||||++---- CLKST
**                      |||+------ IREFST       : Reference range is 31.25kHz to 39.0625kHz
**                      ||+------- DMX32      =0  : FLL factor is 512
**                      ++-------- DRS/DRST   =0  : DCO range is 16MHz to 20MHz
*/

/***********************************************************************************************
**
**  Port I/O
**
**  0x0000  PTAD      Port A Data Register
**  0x0001  PTADD     Port A Data Direction Register
**  0x0002  PTBD      Port B Data Register
**  0x0003  PTBDD     Port B Data Direction Register
**  0x0004  PTCD      Port C Data Register
**  0x0005  PTCDD     Port C Data Direction Register
**  0x0006  PTDD      Port D Data Register
**  0x0007  PTDDD     Port D Data Direction Register
**  0x1840  PTAPE     Port A Pull Enable Register
**  0x1841  PTASE     Port A Slew Rate Enable Register
**  0x1842  PTADS     Port A Drive Strength Selection Register
**  0x1844  PTBPE     Port B Pull Enable Register
**  0x1845  PTBSE     Port B Slew Rate Enable Register
**  0x1846  PTBDS     Port B Drive Strength Selection Register
**  0x1848  PTCPE     Port C Pull Enable Register
**  0x1849  PTCSE     Port C Slew Rate Enable Register
**  0x184A  PTCDS     Port C Drive Strength Selection Register
**  0x184C  PTDPE     Port D Pull Enable Register
**  0x184D  PTDSE     Port D Slew Rate Enable Register
**  0x184E  PTDDS     Port D Drive Strength Selection Register
*/

#define init_PTAD     0b10000000
//                      00000000 = reset
#define init_PTADD    0b10000000
//                      00000000 = reset
#define init_PTAPE    0b00000110
//                      00000000 = reset
#define init_PTASE    0b00000000
//                      00000000 = reset
#define init_PTADS    0b00000000
//                      00000000 = reset
/*                      ||||||||
**                      |||||||+-- X_OUT
**                      ||||||+--- Y_OUT        - MMA845x INT1
**                      |||||+---- Z_OUT        - MMA845x INT2
**                      ||||+----- DIS_MCU
**                      |||+------ BKGD
**                      ||+------- RESET
**                      |+-------- EXTRA_AD
**                      +--------- LEDB_BB      - Blue LED cathode
*/
#define INT1_IS_ACTIVE              (PTAD_PTAD1 == 0)
#define INT2_IS_ACTIVE              (PTAD_PTAD2 == 0)
#define LED_BlueOn                  (PTAD_PTAD7 = 0)
#define LED_BlueOff                 (PTAD_PTAD7 = 1)

#define init_PTBD     0b00100000
//                      00000000 = reset
#define init_PTBDD    0b00100000
//                      00000000 = reset
#define init_PTBPE    0b00000000
//                      00000000 = reset
#define init_PTBSE    0b00000000
//                      00000000 = reset
#define init_PTBDS    0b00000000
//                      00000000 = reset
/*                      ||||||||
**                      |||||||+-- RXD
**                      ||||||+--- TXD
**                      |||||+---- SPSCK        - AT25DF321 SPSCK
**                      ||||+----- MOSI         - AT25DF321 SI
**                      |||+------ MISO         - AT25DF321 SO
**                      ||+------- SS           - AT25DF321 CS
**                      |+-------- SDA          - MMA845x SDA
**                      +--------- SCL          - MMA845x SCL
*/
#define SPI_SS_SELECT               (PTBD_PTBD5 = 0)
#define SPI_SS_DESELECT             (PTBD_PTBD5 = 1)

#define init_PTCD     0b10111011
//                      00000000 = reset
#define init_PTCDD    0b10110111
//                      00000000 = reset
#define init_PTCPE    0b01000000
//                      00000000 = reset
#define init_PTCSE    0b00000000
//                      00000000 = reset
#define init_PTCDS    0b00000000
//                      00000000 = reset
/*                      ||||||||
**                      |||||||+-- LEDG_BB      - Green LED cathode
**                      ||||||+--- SLEEP        - MMA845x CS
**                      |||||+---- G_SEL_2      - MMA845x SA0
**                      ||||+----- G_SEL_1
**                      |||+------ LEDR_BB      - Red LED cathode
**                      ||+------- BUZZER_BB
**                      |+-------- PUSH_BUTTON
**                      +--------- LED_OB       - Yellow LED cathode
*/
#define LED_GreenOn                 (PTCD_PTCD0 = 0)
#define LED_GreenOff                (PTCD_PTCD0 = 1)
//#define SENSOR_SHUTDOWN             (PTCD_PTCD1 = 0) //This can be removed
//#define SENSOR_ACTIVE               (PTCD_PTCD1 = 1) //This can be removed
#define LED_RedOn                   (PTCD_PTCD4 = 0)
#define LED_RedOff                  (PTCD_PTCD4 = 1)
#define LED_YellowOn                (PTCD_PTCD7 = 0)
#define LED_YellowOff               (PTCD_PTCD7 = 1)
#define SA0_PIN                     (PTCD_PTCD2)

#define init_PTDD     0b00000000
//                      00000000 = reset
#define init_PTDDD    0b00000000
//                      00000000 = reset
#define init_PTDPE    0b00000000
//                      00000000 = reset
#define init_PTDSE    0b00000000
//                      00000000 = reset
#define init_PTDDS    0b00000000
//                      00000000 = reset
/*                      xxxx||||
**                          |||+-- PTD0
**                          ||+--- PTD1
**                          |+---- PTD2
**                          +----- PTD3
*/

/***********************************************************************************************
**
**  Interrupt Pin (IRQ)
**
**  0x000F  IRQSC     Interrupt Pin Request Status and Control Register
*/

#define init_IRQSC    0b00000000
/*                      00000000 = reset
**                      x|||||||
**                       ||||||+-- IRQMOD     =0  :
**                       |||||+--- IRQIE      =0  :
**                       ||||+---- IRQACK
**                       |||+----- IRQF
**                       ||+------ IRQPE      =0  :
**                       |+------- IRQEDG     =0  : Falling edge and/or low-level sensitive
**                       +-------- IRQPDD     =0  : Pull device enabled if IRQPE = 1
*/

/***********************************************************************************************
**
**  Keyboard Interrupt (KBI)
**
**  0x000C  KBISC     KBI Interrupt Status and Control Register
**  0x000D  KBIPE     KBI Interrupt Pin Select Register
**  0x000E  KBIES     KBI Interrupt Edge Select Register
**  0x000F  IRQSC
*/

#define init_KBISC    0b00000100
/*                      00000000 = reset
**                      xxxx||||
**                          |||+-- KBIMOD     =0  : Edge detection only
**                          ||+--- KBIE       =0  : KBF interrupt disabled
**                          |+---- KBACK
**                          +----- KBF
*/
#define INT_PINS_ENABLED            (KBISC_KBIE = 1)
#define INT_PINS_DISABLED           (KBISC_KBIE = 0)
#define CLEAR_KBI_INTERRUPT         (KBISC_KBACK = 1)

#define init_KBIPE    0b00000110
/*                      00000000 = reset
**                      ||||||||
**                      |||||||+-- KBIPE0     =0  : PTA0 KBI interrupt disabled
**                      ||||||+--- KBIPE1     =1  : PTA1 KBI interrupt enabled
**                      |||||+---- KBIPE2     =1  : PTA2 KBI interrupt enabled
**                      ||||+----- KBIPE3     =0  : PTA3 KBI interrupt disabled
**                      |||+------ KBIPE4     =0  : PTB0 KBI interrupt disabled
**                      ||+------- KBIPE5     =0  : PTB1 KBI interrupt disabled
**                      |+-------- KBIPE6     =0  : PTB2 KBI interrupt disabled
**                      +--------- KBIPE7     =0  : PTB3 KBI interrupt disabled
*/
#define INT1_PIN_ENABLED            (KBIPE_KBIPE1 = 1)
#define INT1_PIN_DISABLED           (KBIPE_KBIPE1 = 0)
#define INT2_PIN_ENABLED            (KBIPE_KBIPE2 = 1)
#define INT2_PIN_DISABLED           (KBIPE_KBIPE2 = 0)
#define intPinEnableRegister        (KBIPE)

#define init_KBIES    0b00000000
/*                      00000000 = reset
**                      ||||||||
**                      |||||||+-- KBEDG0     =0  : PTA0 KBI pullup
**                      ||||||+--- KBEDG1     =0  : PTA1 KBI pullup
**                      |||||+---- KBEDG2     =0  : PTA2 KBI pullup
**                      ||||+----- KBEDG3     =0  : PTA3 KBI pullup
**                      |||+------ KBEDG4     =0  : PTB0 KBI pullup
**                      ||+------- KBEDG5     =0  : PTB1 KBI pullup
**                      |+-------- KBEDG6     =0  : PTB2 KBI pullup
**                      +--------- KBEDG7     =0  : PTB3 KBI pullup
*/
#define INT1_FALLING                (KBIES_KBEDG1 = 0)
#define INT1_RISING                 (KBIES_KBEDG1 = 1)
#define INT2_FALLING                (KBIES_KBEDG2 = 0)
#define INT2_RISING                 (KBIES_KBEDG2 = 1)
#define INT_BOTH_FALLING            (KBIES = 0x00)
#define INT_BOTH_RISING             (KBIES = 0x06)

/***********************************************************************************************\
* Public memory declarations
\***********************************************************************************************/

#pragma DATA_SEG __SHORT_SEG _DATA_ZEROPAGE

extern BIT_FIELD SystemFlag;                // system control flags
extern byte full_scale;                     // current accelerometer full scale setting
extern int deviceID;

#pragma DATA_SEG DEFAULT

/***********************************************************************************************\
* Public prototypes
\***********************************************************************************************/

extern void InterruptsActive (byte ctrl_reg3, byte ctrl_reg4, byte ctrl_reg5);

#endif  /* _SYSTEM_H_ */

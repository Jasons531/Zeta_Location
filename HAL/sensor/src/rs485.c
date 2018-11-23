/*
**************************************************************************************************************
*	@file			rs485.c
*	@author 	Jason
*	@version  V0.1
*	@date     2018/07/14
*	@brief		用RS485使用查询方式
***************************************************************************************************************
*/ 
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rs485.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*********************************************************************
 * RS485/串口相关配置
 */
#define RS485_TO_TX()	 HAL_GPIO_WritePin(Out_485_DE_Pin_GPIO_Port,Out_485_DE_Pin_Pin, GPIO_PIN_SET);	// RS485总线切换到发送模式
#define RS485_TO_RX()	 HAL_GPIO_WritePin(Out_485_DE_Pin_GPIO_Port,Out_485_DE_Pin_Pin, GPIO_PIN_RESET);// RS485总线切换到接收模式

rs485_t Rs485s;
	
/*
 *	Rs485Init:		波特率9600，发送和接收都为DMA模式
 *	参数：			  无
 *	返回值：		  无	
 */
void Rs485Init(void)
{
	MX_USART5_UART_Init(  );
	InitUartFifo(  );
	
	Rs485s.PinInit 		= Rs485PinInit;
	Rs485s.OpenPin 		= Rs485OpenPin;
	Rs485s.ClosePin 	= Rs485ClsoePin;
	Rs485s.PowerOn 		= _12VPowerOn;
	Rs485s.PowerOff 	= _12VPowerOff;
	Rs485s.Print			= Rs485Print;
	Rs485s.Crc16			= CalcCRC16;
	Rs485s.GetData   	= Rs485GetData;
	Rs485s.Cmd				= Rs485Cmd;
	
	Rs485s.PinInit(  );
}

/*
 *	Rs485PinInit:		初始化传感器
 *	参数：					无
 *	返回值：				无	
 */
void Rs485PinInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_InitStruct.Pin = Out_12V_ON_Pin_Pin|RS485PIN_0|RS485PIN_1|RS485PIN_2;                     
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Out_485_DE_Pin_Pin|POWER_485IC_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Out_485_DE_Pin_GPIO_Port, &GPIO_InitStruct);
}

/*
 *	Rs485OpenPin:		使能485接口
 *	参数：					打开相应的485接口
 *	返回值：				无	
 */
void Rs485OpenPin(int index)
{
	uint16_t pin =  RS485PIN_0;
	for(uint8_t i = 0 ; i < 3 ; i ++)
	{			
		HAL_GPIO_WritePin(GPIOB,pin << i,GPIO_PIN_RESET);
	}
	HAL_GPIO_WritePin(GPIOB,pin << index ,GPIO_PIN_SET);
}


/*
 *	Rs485ClsoePin:	失能485接口
 *	参数：					关闭所有485接口
 *	返回值：				无	
 */
void Rs485ClsoePin(void)
{
	uint16_t pin = RS485PIN_0;
	for(int i = 0 ; i < 3 ; i ++)
	{			
     HAL_GPIO_WritePin(GPIOB,pin << i,GPIO_PIN_RESET);
  }
}

/*
 *	_12VPowerOn:		使能12V电源
 *	参数：					无
 *	返回值：				无	
 */
void _12VPowerOn(void)
{
	HAL_GPIO_WritePin(Out_12V_ON_Pin_GPIO_Port,Out_12V_ON_Pin_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(POWER_485IC_Port,POWER_485IC_Pin,GPIO_PIN_SET);
}

/*
 *	_12VPowerOff:	  关闭12V电源
 *	参数：					无
 *	返回值：				无	
 */
void _12VPowerOff(void)
{	
	HAL_GPIO_WritePin(Out_12V_ON_Pin_GPIO_Port,Out_12V_ON_Pin_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(POWER_485IC_Port,POWER_485IC_Pin,GPIO_PIN_RESET);
}

/*
 *	Rs485GetData:	获取485返回数据
 *	data:					存取数据缓存
*  debuglevel:    debug等级
 *	返回值：	    数据长度
 */
uint8_t Rs485GetData(uint8_t *data, uint8_t debuglevel)
{	
	uint8_t ch = 0;
	uint8_t length = 0;
	    
	DEBUG(debuglevel,"%s,----get data----",__FILE__);

	while(FIFO_UartReadByte(&usart_rs485,&ch) == HAL_OK)	
	{			
		data[length] = ch;		
		DEBUG(debuglevel,"%02X ",data[length]);	
	  length++;
	}
	DEBUG(debuglevel,"\r\n");
	return length;
}


/*
 *	Rs485Cmd:	Rs485下发命令
 *	sendData:	命令数据
 *	len			：命令长度
*  debuglevel: debug等级
 *  time_out: 等待时间
 *	返回值：	接收到Rs485数据长度
 */
uint8_t Rs485Cmd(uint8_t *sendData, uint8_t len, uint8_t debuglevel, uint32_t time_out)
{	
		uint8_t temp[20] = {0};
    RS485_TO_TX();		 
    Rs485s.Crc16(sendData,len);
		
  	HAL_Delay(time_out);
//    DEBUG(2,"---send : ");
//    for(int i = 0; i < len+2; i++)
//    DEBUG(2,"%02X ",sendData[i]);
//    DEBUG(2,"\r\n");
    HAL_UART_Transmit(&huart5,sendData,len + 2,0xffff);		
	
    RS485_TO_RX(  );
	
	  memset(Rs485s.Revbuff, 0, 20);
	
		if(sendData[0] == 0xFD)
		{
			HAL_Delay(200);
		}
		else
    HAL_Delay(NBI_RS485_REV_TIME_OUT);
								
    uint8_t length = Rs485s.GetData(temp,debuglevel);
		
		memcpy1(Rs485s.Revbuff,temp,length);
    
    char crcH = Rs485s.Revbuff[length-1];
    char crcL = Rs485s.Revbuff[length-2];
    
    Rs485s.Crc16(Rs485s.Revbuff,length-2);
    if(crcH == Rs485s.Revbuff[length-1] && crcL == Rs485s.Revbuff[length-2])			
        return length;
    else
        return 0;		
}

/*
 *	Rs485Print:	打印485数据
 *	buff:			  数据缓存
 *	len			：	数据长度
 *	返回值：	  无
 */
void Rs485Print(uint8_t *buff,int len, uint8_t DebugLive)
{
	DEBUG(DebugLive,"%s,%d:RS485 Revdata: ",__FILE__, __LINE__);

	for(int i = 0 ; i < len ; i++)
	{
		DEBUG(DebugLive,"%02X ",buff[i]);
	}
	DEBUG(DebugLive,"\r\n");	
}

/*
 *	CalcCRC16:	计算CRC16校验值
 *	data:		数据指针
 *	len:		数据长度
 *	返回值：	16位的CRC校验值
 */
uint16_t CalcCRC16(uint8_t *data, uint8_t len)
{
	uint16_t result = 0xffff;
	uint8_t i, j;

	for (i=0; i<len; i++)
	{
		result ^= data[i];
		for (j=0; j<8; j++)
		{
			if ( result&0x01 )
			{
					result >>= 1;
					result ^= 0xa001;
			}
			else
			{
					result >>= 1;
			}
		}
	}
	GET_CRC(&(data[len]), result);
	
	return result;
}

/*
*memcpy1：	数据拷贝
*dst：			拷贝目标
*src:				拷贝源地址
*size:			数据大小
*返回：			无
*/
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
	while( size-- )
	{
		*dst++ = *src++;
	}
}

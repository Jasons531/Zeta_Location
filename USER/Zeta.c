/*
**************************************************************************************************************
*	@file		Zeta.c
*	@author Jason
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "Zeta.h"
#include "usart.h"
#include "Debug.h"

Zeta_t 							ZetaRecviceBuf = {0, 0, {0}, {0}, 0, Reset, true};
Zeta_t 							ZetaSendBuf		 = {0, 0, {0}, {0}, 0, Reset, true};
ZetaTimer_t					ZetaTimer;
const ZetaHandle_t 	ZetaHandle = {ZetaInit, ZetaPowerOn, ZetaPowerOff, WakeupZetaEnable, WakeupZetaDisable, \
																	ZetaInterrupt, ZetaSend, ZetaRecv, CalcCRC8, ZetaStatus, ZetaDownCommand};

/*ZetaInit：初始化Zeta IO
*参数：			无ZetaHandle.PowerOn
*返回值：   无
*/
void ZetaInit(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE(); 
	__HAL_RCC_GPIOB_CLK_ENABLE();       

	///对于低电平触发中断，需要把GPIO设置为上拉，然后下降沿触发，反之高电平触发，反过来
	GPIO_Initure.Pin=ZETAINT_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      			
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(ZETAINT_IO,&GPIO_Initure);
			
	GPIO_Initure.Pin=ZETAWAKUP_PIN|ZETAPOWER_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
	GPIO_Initure.Pull=GPIO_PULLUP;          
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
	HAL_GPIO_Init(ZETAPOWER_IO,&GPIO_Initure);
	
	GPIO_Initure.Pin=ZETAPOWER_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
	GPIO_Initure.Pull=GPIO_PULLUP;          
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
	HAL_GPIO_Init(ZETAPOWER_IO,&GPIO_Initure);
	
	GPIO_Initure.Pin=ZETASTATU_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_INPUT;  
	GPIO_Initure.Pull=GPIO_PULLUP;          
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
	HAL_GPIO_Init(ZETASTATU_IO,&GPIO_Initure);
	
	HAL_GPIO_WritePin(ZETAWAKUP_IO,ZETAWAKUP_PIN,GPIO_PIN_SET);	
					
	//中断线2-PC2
	HAL_NVIC_SetPriority(EXTI0_1_IRQn,2,0);       //抢占优先级为2，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);  
	
	MX_LPUART1_UART_Init(  ); 
}

/*ZetaPowerOn：打开Zeta电源
*参数：			   无
*返回值：      无
*/
void ZetaPowerOn(void)
{
	HAL_GPIO_WritePin(ZETAPOWER_IO,ZETAPOWER_PIN,GPIO_PIN_SET);	
}

/*ZetaPowerOff: 关闭Zeta电源
*参数：			    无
*返回值：       无
*/
void ZetaPowerOff(void)
{
	HAL_GPIO_WritePin(ZETAPOWER_IO,ZETAPOWER_PIN,GPIO_PIN_RESET);	
}

/*WakeupZetaEnable：唤醒Zeta,下降沿有效Status IO转换为高电平，1MS后串口可以使用
*参数：							无
*返回值：   				无
*/
void WakeupZetaEnable(void)
{
	HAL_GPIO_WritePin(ZETAWAKUP_IO,ZETASTATU_PIN,  GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ZETAWAKUP_IO,ZETAWAKUP_PIN,  GPIO_PIN_RESET);
}

/*WakeupZetaDisable：Zeta进入休眠
*参数：							 无
*返回值：   				 无
*/
void WakeupZetaDisable(void)
{
	HAL_GPIO_WritePin(ZETAWAKUP_IO,ZETAWAKUP_PIN,  GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(ZETAWAKUP_IO,ZETAWAKUP_PIN,GPIO_PIN_SET);
}

/*ZetaInterrupt：Zeta接收数据脉冲
*参数：					 无
*返回值：   		 无
*/
void ZetaInterrupt(void)
{	
	///读取数据
	HAL_Delay(160); ///延时 = Zeta串口稳定接收数据 + 串口超时时间
	
	ZetaHandle.Recv(  );
	
	DEBUG(2,"%s\r\n",__func__);
}

/*ZetaDownCommand：Zeta下行命令处理
*参数：					 	 下行数据
*返回值：   		 	 无
*/
uint8_t ZetaDownCommand(uint8_t *RevBuf)
{
	uint8_t state = 0xFE;
	
	switch( RevBuf[0] )
	{
		case 0xA0: ///修改采样周期
			if( 0x00 == ZetaHandle.CRC8( RevBuf,4 ) )
			{
				uint32_t data = 0;
				data |= RevBuf[1] << 4;
				data |= RevBuf[2];
								
				////data write in flash			
				state = FlashWrite32( SLEEP_ADDR, &data, 1 );
								
				User.SleepTime =	FlashRead32(SLEEP_ADDR);
								
				DEBUG_APP(2,"----data----%d User.SleepTime = %d",data,User.SleepTime);
			}
			
		break;
			
		case 0xA1: ///重新上报GPS位置信息
			if( 0x00 == ZetaHandle.CRC8( RevBuf,3 ) )
			{
				SetGpsMode.ReStart = true;
				SetGpsMode.LocationState = PATIONNULL;
				state = 0x01;
			}
			else
				state = 0xFE;
			break;
			
		case 0xA2:
			if( 0x00 == ZetaHandle.CRC8( RevBuf,3 ) )
			{
			  ZetaSendBuf.MaxLen = RevBuf[1];
			  FlashWrite16( MAXLEN_ADDR, (uint16_t *)ZetaSendBuf.MaxLen, 1 );
				
				ZetaSendBuf.MaxLen -= FIXLEN;
				
				DEBUG_APP(2,"---MaxLen--- %d\r\n",ZetaSendBuf.MaxLen);
				
				state = 0x01;
			}
		
		break;
			
		case 0xA3:  ///获取MAC地址
			if( 0x00 == ZetaHandle.CRC8( RevBuf,3 ) )
			{			
				state = 0x03;
			}
			else
				state = 0xfc;
		
		break;
			
		case 0xA4:  ///进入水产模式
			if( 0x00 == ZetaHandle.CRC8( RevBuf,3 ) )
			{			
				uint32_t data = RevBuf[1];
				state = FlashWrite32( AQUATIC_MODE_ADDR, &data, AQUATIC_MODE_SIZE );
			}
		
		break;
		
		default:
			break;
	}
	
	return state;
}


/*ZetaStatus：Zeta串口状态
*参数：				无
*返回值：   	无
*/
GPIO_PinState ZetaStatus(void)
{
	return HAL_GPIO_ReadPin(ZETASTATU_IO,ZETASTATU_PIN);
}

/*ZetaSend：Zeta发送数据
*参数：			缓存数据集
*返回值：   无
*/
void ZetaSend(Zeta_t *ZetaBuf)
{
	HAL_UART_Transmit(&hlpuart1, ZetaBuf->Buf, ZetaBuf->Len, 0xFFFFFFFF);
	ZetaRecviceBuf.States = Reset;
}

/*ZetaRecv：获取Zeta数据
*参数：			无
*返回值：   Zeta工作状态
*/
ZetaState_t ZetaRecv(void)
{
	ZetaRecviceBuf.States = Reset;
	
	if( HAL_GetTick(  )-ZetaRecviceBuf.Uart_time > 40 && UART_RX_LPUART1.Rx_State) 
	{
		UART_RX_LPUART1.Rx_State = false;
		
		memcpy(ZetaRecviceBuf.Buf,UART_RX_LPUART1.USART_RX_BUF,UART_RX_LPUART1.USART_RX_Len);
		ZetaRecviceBuf.Len = UART_RX_LPUART1.USART_RX_Len;
		
		for(uint8_t i = 0; i < ZetaRecviceBuf.Len; i++)
		{
			printf("%02x ",UART_RX_LPUART1.USART_RX_BUF[i]);
		}
		DEBUG(2,"\r\n");

		memset(UART_RX_LPUART1.USART_RX_BUF, 0, UART_RX_LPUART1.USART_RX_Len);
		
		UART_RX_LPUART1.USART_RX_Len = 0;
		
		DEBUG_APP(2,"---ZetaRecviceBuf: ");
		for(uint8_t i = 0; i < ZetaRecviceBuf.Len; i++)
		printf("%02x ",ZetaRecviceBuf.Buf[i]);
		DEBUG(2,"\r\n");
		
		if(ZetaRecviceBuf.Len == 4) ///设置状态反馈
		{
			if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x01 )
				ZetaRecviceBuf.States = DataAck;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x02 )
				ZetaRecviceBuf.States = BufFull;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x03 )
				ZetaRecviceBuf.States = LenError;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x20 )
				ZetaRecviceBuf.States = SetSecess;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x21 )
				ZetaRecviceBuf.States = SetFail;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x40 )
				ZetaRecviceBuf.States = InvaildWarning;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x41 )
				ZetaRecviceBuf.States = StopPrompt;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x42 )
				ZetaRecviceBuf.States = SendDatafalse;
			else if( ZetaRecviceBuf.Buf[2] == 0x04 && ZetaRecviceBuf.Buf[3] == 0x43 )
				ZetaRecviceBuf.States = Unregistered;
		}
		else if(ZetaRecviceBuf.Len >= 5 && ZetaRecviceBuf.Buf[0] == 0xff && ZetaRecviceBuf.Buf[1] == 0x00 )
		{
			ZetaRecviceBuf.States = Payload;
		}
		
		////下行控制数据处理:FF 00 08 30 23 45 67 89：发送数据周期、唤醒MCU发送等 ff 00 len 30 data
		
		if( ZetaRecviceBuf.Buf[0] == 0xFF && ZetaRecviceBuf.Buf[1] == 0x00 && ZetaRecviceBuf.Buf[3] == 0x30 )
		{
			memset(ZetaRecviceBuf.RevBuf, 0, ZetaRecviceBuf.Len);
			memcpy(ZetaRecviceBuf.RevBuf, &ZetaRecviceBuf.Buf[4],ZetaRecviceBuf.Len-4);

			for(uint8_t i = 0; i < ZetaRecviceBuf.Len - 4; i++)
			printf("%02x ",ZetaRecviceBuf.RevBuf[i]);
			DEBUG(2,"\r\n");
		}

		DEBUG(3,"States: %02x\r\n",ZetaRecviceBuf.States);
		memset(ZetaRecviceBuf.Buf, 0, ZetaRecviceBuf.Len);
		ZetaRecviceBuf.Len = 0;
	}
		return ZetaRecviceBuf.States;
}

/*
 *	CalcCRC8:	计算CRC8校验值
 *	ptr:			数据指针
 *	len:			数据长度
 *	返回值：	8位的CRC校验值
 */
uint8_t CalcCRC8(uint8_t *ptr, uint8_t len)
{
	uint8_t i; 
	uint8_t crc=0x00; /* 计算的初始crc值 */ 

	while(len--)
	{
		crc ^= *ptr++;  /* 每次先与需要计算的数据异或,计算完指向下一数据 */  
		for (i=8; i>0; --i)   /* 下面这段计算过程与计算一个字节crc一样 */  
		{ 
			if (crc & 0x80)
			crc = (crc << 1) ^ 0x31;
			else
			crc = (crc << 1);
		}
	}
	
	DEBUG_APP(3,"crc = %02x\r\n",crc);
	return (crc); 
}


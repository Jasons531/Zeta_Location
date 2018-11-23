#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "stm32l0xx_hal.h"

#define DEV_ADDR											0x0801ffe0			//设备ID存储地址
#define DEV_ADDR_SIZE									0x12				//设备ID占用存储空间

#define SLEEP_ADDR										0x0801ffc0			//休眠时间存储地址 
#define SLEEP_ADDR_SIZE								0x04				//休眠时间占用存储空间

#define MAXLEN_ADDR										0x0801ffbc			//Zeta数据长度存储地址
#define MAXLEN_ADDR_SIZE							0x01				//Zeta数据长度占用存储空间

#define AQUATIC_MODE_ADDR 						0x0801ffb8   //水产工作模式选择
#define AQUATIC_MODE_SIZE							0x01				

#define	HEART_CYCLE_ADDR							0x0801ffb4	///心跳地址

#define	ALARM_CYCLE_ADDR							0x0801ffb0	///报警地址

#define	GPS_LOCA_TIME_ADDR						0x0801ffac	///GPS定位超时时间

#define	MOVE_CONDITION_ADDR						0x0801ffa8	///移动条件

#define MOVE_STOP_CONDITION_ADDR			0x0801ffa4	///移动停止条件

#define MOVE_ENABLE_ADDR							0x0801ffa0	///移动使能条件


#define FLAG_ADD											0x0801fd10			//备份寄存器的代替位置，用于记录各种标识位
#define	FLAG_ADD_SIZE									0x10


#define FLAG	FlashRead32(FLAG_ADD)		

typedef enum FLAGTYPE						//标志类型,最大值为31
{
	SOFT_RESET=0x00,						//软件复位标志
	UPDATE_RESET,							//更新固件标志
	DOWNLINK_CONFIRM,						//下行确认标志,用于回复服务器的下行控制命令
	CLEAN_FLASH,							//复位所有数据Flash中的数据
	GET_LOCATION,							//获取位置信息
	UPDATE_SUCCEED,
	UPDATE_FAIL,
  CLOSE_IWDG,                             //关闭独立看门狗
	END_OF_FLAGTYPE
}FlagType;

/*
 *	FlashReadPage:		读取1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		保存读取到的数据的指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashReadPage(uint32_t PageAddr, uint32_t *pBuffer);
/*
 *	FlashWritePage:		写1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		用于写入Flash中的数据指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashWritePage( uint32_t PageAddr, uint32_t *pPageBuffer);

/*
 *	FlashWrite32:		写4字节(32位)数据
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小/4)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite32( uint32_t WriteAddr, uint32_t * pBuffer, uint16_t NumToWrite );
/*
 *	FlashWrite16:		写2字节(16位)数据
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小/2)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite16( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite );

/*
 *	FlashRead32:		读取4字节(32位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint32_t FlashRead32(uint32_t ReadAddr );
/*
 *	FlashRead16:		读取2字节(16位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint16_t FlashRead16(uint32_t ReadAddr );

/*
 *	FlashRead8:		读取1字节(8位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	返回值：		返回读取到的数据		
 */
uint8_t FlashRead8(uint32_t ReadAddr );
/*
 *	FlashReadChar:	读取字节(8位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		实际读取到的字符长度,读取错误返回0
 */
uint16_t FlashReadChar(uint32_t ReadAddr,char* pBuffer,uint16_t NumToRead);
/*
 *	FlashRead16More:	读取多个2字节(16位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		1成功 0失败
 */
uint16_t FlashRead16More(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);

/*
 *	ReadSerialNumber:	从flash读取序列号
 * 	device_id:			设备ID
 *	stream:				数据流
 *	server_add:			服务器IP地址
 *	server_port:		服务器端口
 *	返回值：				读取成功则返回0，否则返回1
 */
uint8_t ReadSerialNumber(char *device_id,char *stream,char *server_addr,char *server_port);

/*
 *	SetFlag:			修改FLAG_ADD地址的值，设置相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void SetFlag(FlagType flag);

/*
 *	CleanFlag:			修改FLAG_ADD地址的值，清除相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void CleanFlag(FlagType flag);

/*
 *	CheckFlag:			查询相关标志位是否被标志
 *	参数flag：			只能为FlagType类型
 *	返回值：			被标志返回>0的值，未标志返回0
 */
uint8_t CheckFlag(FlagType flag);

#endif	


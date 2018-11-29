/*
**************************************************************************************************************
*	@file			mma8452q.c
*	@author 	Jason
*	@version 	V0.1
*	@contact  Jason_531@163.com
* @date    	2018/10/29
*	@brief	  加速度传感器处理库
***************************************************************************************************************
*/
#include "mma8452q.h"
#include "i2c.h"

///瞬态检测
#define		TRANSIENT				0

//运动检测
#define		MOTION					1

static tword x_value;                       // 16-bit X accelerometer value
static tword y_value;                       // 16-bit Y accelerometer value
static tword z_value;                       // 16-bit Z accelerometer value

/*
*IIC_RegRead：读取寄存器状态
*参数				：地址、寄存器
*返回				：寄存器数据
*/
uint8_t IIC_RegRead(uint16_t address, uint8_t reg)
{
	uint8_t redata = 0;
	
	HAL_I2C_Mem_Read(&hi2c2, address+1, reg, 1, &redata, 1, 10);
	
	return redata;
}

/*
*IIC_RegWrite	 : 写寄存器
*参数					 ：地址、寄存器
*返回					 ：无
*/
void IIC_RegWrite(uint16_t address, uint8_t reg, uint8_t data)
{
	HAL_I2C_Mem_Write(&hi2c2, address, reg, 1, &data, 1, 10);
}

/* MMA8452InterruptPinInit：MMA8452q外部唤醒IO初始化，只使用MMA8492 PIN1
*参数：											无
*返回值：   								无
*/
void MMA8452InterruptPinInit(void)
{						
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE(); 
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE(); 

#if 0	
	
	GPIO_InitStruct.Pin = MMA8452INT_WAKE;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; 
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(MMA8452INT_WAKE_IO, &GPIO_InitStruct);
	
#endif
	
	GPIO_InitStruct.Pin = MMA8452INT_1;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; //GPIO_MODE_IT_FALLING
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(MMA8452INT_1_IO, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 8, 0);	
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);		
}

/*
*MMA845xInit：初始化ODR、power_mode
*参数			：无
*返回			：无
*/
void MMA845xInit (void)
{
	MX_I2C2_Init(  );
	
	MMA8452InterruptPinInit(  );
		
	MMA8452xSetPowerMode( 0x1F );  ///Auto-Sleep+Low_Power
	
	MMA845xSetDataRate( 0x40 | DATA_RATE_80MS ); ///Sleep_Freq + 12.5hz

	MMA845xSetPassFilter( HPF_OUT_MASK );///高通or低通滤波

	MMA845xEnterActiveG(FULL_SCALE_2G);

	MMA8452xInterrupt(  );

	MMA845xID(  );
}

/*
*MMA845xStandby：设置传感器进入休眠
*参数					 ：无
*返回					 ：无
*/
void MMA845xStandby (void)
{
  uint8_t n;
  /*
  **  Read current value of System Control 1 Register.
  **  Put sensor into Standby Mode.
  **  Return with previous value of System Control 1 Register.
  */
  n = IIC_RegRead(MA8452Q_ADDR, CTRL_REG1);
  IIC_RegWrite(MA8452Q_ADDR, CTRL_REG1, n & ~ACTIVE_MASK);
}

/*
*MMA845xActive：退出休眠
*参数					：无
*返回					：无
*/void MMA845xActive(void)
{
	IIC_RegWrite(MA8452Q_ADDR, CTRL_REG1, (IIC_RegRead(MA8452Q_ADDR, CTRL_REG1) | ACTIVE_MASK));
}

/*
*MMA845xID：获取传感器ID
*参数			：无
*返回			：无
*/
void MMA845xID(void)
{
	uint8_t wdata = MA8452Q_ID;
	
	uint8_t rdata = 0;
				
	HAL_I2C_Mem_Read(&hi2c2,MA8452Q_ADDR+1,wdata,1,&rdata,1,10);  //////适合16bit寄存器	
	DEBUG(2,"MMA845xID = %02X\r\n", rdata);

	if(rdata != 0x2A)
		DEBUG_APP(2,"MMA845x Init Error");
}

/*
*MMA8452xSetPowerMode：设置速率
*参数							 	 ：00: normal 1: Low Noise Low Power 2: High Resolution 3: Low Power
*返回							 	 ：无
*/
void MMA8452xSetPowerMode(uint8_t	PowerMode)
{
	MMA845xStandby(  );
	
	IIC_RegWrite(MA8452Q_ADDR, CTRL_REG2, 0X40 | PowerMode); //RST: 否则可以存在异常
	
	MMA845xActive(  );
}

/*
*MMA845xSetDataRate：设置速率
*参数							 ：频率 - DATA_RATE_80MS(12.5hz)
*返回							 ：无
*/
void MMA845xSetDataRate(uint8_t DataRateValue)
{
	MMA845xStandby(  );
	
	IIC_RegWrite(MA8452Q_ADDR, CTRL_REG1, (IIC_RegRead(MA8452Q_ADDR, CTRL_REG1) & ~DR_MASK));
	
	IIC_RegWrite(MA8452Q_ADDR, CTRL_REG1, (IIC_RegRead(MA8452Q_ADDR, CTRL_REG1) | DataRateValue)); 
	
	MMA845xActive(  );
}

/*
*MMA845xSetPassFilter	 ：设置滤波方式
*参数							 		 ：滤波模式
*返回							 		 ：无
*/
void MMA845xSetPassFilter(uint8_t Filtered)
{
	MMA845xStandby(  );
	
	IIC_RegWrite(MA8452Q_ADDR, XYZ_DATA_CFG_REG, (IIC_RegRead(MA8452Q_ADDR, XYZ_DATA_CFG_REG) | Filtered));
		
	MMA845xActive(  );
}

/*
*MMA8452MultipleRead：获取加速度坐标
*参数								：无
*返回								：无
*/
void MMA8452MultipleRead(void)
{
	uint8_t wdata = 0x01;
	uint8_t rdata[6] = {0};
	uint8_t ZYXDR = 0;
	
	uint16_t Xdata, Ydata, Zdata = 0;
	MMA845xActive(  );

	ZYXDR = IIC_RegRead(MA8452Q_ADDR,STATUS_00_REG);
	
	DEBUG(2,"ZYXDR11 = %02x \r\n",ZYXDR);
	
	if(ZYXDR&0x08)
	{	
		for(uint8_t i = 0; i < 6; ++i)
		HAL_I2C_Mem_Read(&hi2c2,MA8452Q_ADDR+1,wdata+i,1,&rdata[i],1,100);  //////适合16bit寄存器	
					
		///12bit 处理方式
		Xdata = (rdata[0] << 4) | (rdata[1] >> 4);
				
		if(Xdata>0x0800) /////负数：无符号表达
		{
			Xdata = 0x0f00 - 0x0800 + 1;
			DEBUG(2,"X = -%.3f ",Xdata * 0.001);
		}
		else
			DEBUG(2,"X = %.3f ",Xdata * 0.001);
		
		Ydata = (rdata[2] << 4) | (rdata[3] >> 4);
				
		if(Ydata>0x0800) /////负数：无符号表达
		{
			Ydata = 0x0f00 - 0x0800 + 1;
			DEBUG(2,"Y = -%.3f ",Ydata * 0.001);
		}
		else
			DEBUG(2,"Y = %.3f ",Ydata * 0.001);

		Zdata = (rdata[4] << 4) | (rdata[5] >> 4);
		
		if(Zdata>0x0800) /////负数：无符号表达
		{
			Zdata = 0x0f00 - 0x0800 + 1;
			DEBUG(2,"Z = -%.3f \r\n",Zdata * 0.001);
		}
		else
			DEBUG(2,"Z = %.3f \r\n",Zdata * 0.001);
		
		DEBUG(2,"data: %d %d %d %d %d %d \r\n",rdata[0],rdata[1],rdata[2],rdata[3],rdata[4],rdata[5]);
		
		ZYXDR = IIC_RegRead(MA8452Q_ADDR,STATUS_00_REG);
	
		DEBUG(2,"ZYXDR22 = %02x \r\n",ZYXDR);
		
		///Read Interrupt Source
		uint8_t data_temp = IIC_RegRead(MA8452Q_ADDR, INT_SOURCE_REG);

		if(data_temp==0x04)
		{
		
			//Read the Motion/Freefall Function to clear the interrupt
			uint8_t data_temp2 = IIC_RegRead(MA8452Q_ADDR, FF_MT_SRC_1_REG);
			
			DEBUG_APP(2,"data_temp = %02x data_temp2 = %02x\r\n",data_temp,data_temp2);
		}
		else if(data_temp==0x20)
		{
			uint8_t data_temp3 =IIC_RegRead(MA8452Q_ADDR, 0X1E);
			DEBUG_APP(2,"data_temp = %02x data_temp3 = %02x\r\n",data_temp,data_temp3);
		}
		
	}
}

/*
*MMA845xCorrectReg：X、Y、Z校准
*参数								：无
*返回								：无
*/
void MMA845xCorrectReg(void)
{
	uint8_t wdata = 0x01;
	uint8_t value[6] = {0};
	
	int8_t X_cal=0;
	int8_t Y_cal=0;
	int8_t Z_cal=0;
	
	MMA845xStandby();
	
	//OFF_X_REG
	IIC_RegWrite(MA8452Q_ADDR, OFF_X_REG, 0X00);
	
	//OFF_Y_REG
	IIC_RegWrite(MA8452Q_ADDR, OFF_Y_REG, 0X00);
	
	//OFF_Z_REG
	IIC_RegWrite(MA8452Q_ADDR, OFF_Z_REG, 0X00);
	
	MMA845xActive(  );
	
	HAL_Delay(30); ///2/ODR + 1ms delay timing

	for(uint8_t i = 0; i < 6; ++i)
	HAL_I2C_Mem_Read(&hi2c2,MA8452Q_ADDR+1,wdata+i,1,&value[i],1,100);  //////适合16bit寄存器	
	
	x_value.Byte.hi = value[0];
	x_value.Byte.lo = value[1];
	y_value.Byte.hi = value[2];

	y_value.Byte.lo = value[3];
	z_value.Byte.hi = value[4];
	z_value.Byte.lo = value[5];
		
	if (x_value.Byte.hi> 0x7F)
	{
		x_value.Word= (~x_value.Word +1)>>4;
		X_cal= X_cal + x_value.Word/2;
		}
		else
		{
		X_cal= X_cal + ((~x_value.Word+1)>>4)/2;
		}
		if (y_value.Byte.hi> 0x7F)
		{
		y_value.Word= (~y_value.Word +1)>>4;
		Y_cal= Y_cal + y_value.Word/2;
		}
		else
		{
		Y_cal= Y_cal + ((~y_value.Word+1)>>4)/2;
		}
		if (z_value.Byte.hi> 0x7F)
		{
		z_value.Word= (~z_value.Word +1)>>4;
		Z_cal+=(1024 + z_value.Word)/2;
		}
		else
		{
		Z_cal+=(int)(1024- (z_value.Word>>4))/2;
		if (Z_cal<0)
		{
		Z_cal+=256;
		}
	}
		
	MMA845xStandby();
	IIC_RegWrite(SlaveAddressIIC, OFF_X_REG, X_cal);
	IIC_RegWrite(SlaveAddressIIC, OFF_Y_REG, Y_cal);
	IIC_RegWrite(SlaveAddressIIC, OFF_Z_REG, Z_cal);
	MMA845xActive();
}

/*
*MMA845xEnterActive2G	：Enter Active g模式
*参数									：无 
*返回									：无
*/
void MMA845xEnterActiveG(uint8_t FullG)
{
	MMA845xStandby();
	
	IIC_RegWrite(SlaveAddressIIC, XYZ_DATA_CFG_REG, (IIC_RegRead(SlaveAddressIIC, XYZ_DATA_CFG_REG) & ~FS_MASK));
	IIC_RegWrite(SlaveAddressIIC, XYZ_DATA_CFG_REG, (IIC_RegRead(SlaveAddressIIC, XYZ_DATA_CFG_REG) | FullG)); 
	
	MMA845xActive();
}

/*
*运动检测、瞬态检测只能二选一，不能同时支持，同时支持暂没调通
*/
void MMA8452xInterrupt(void)
{
	MMA845xStandby(  );

#if MOTION

///运动检测设置START
	IIC_RegWrite(MA8452Q_ADDR, FF_MT_CFG_1_REG, 0x78); //Enable Latch, Freefall, X-axis, Y-axis and Z-axis ：运动检测

	IIC_RegWrite(MA8452Q_ADDR, FT_MT_THS_1_REG, 0x80 | 0x11); ///越高，灵敏度越高
	//阈值寄存器0~127，阈值的最低分辨率为0.063g/LSB. 1.0g/0.063g=15.87. 四舍五入为16，阈值设置为10H
	
	IIC_RegWrite(MA8452Q_ADDR, FF_MT_COUNT_1_REG, 0x03); //12.5hz 80 ms debounce timing 240ms
///运动检测设置END

	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG3, 0X0A);// PP_OD_MASK 注意：IPOL = 0 :该位代表中断脉冲当前是一直处于高电平，MCU端采用下降沿触发，否则失败
	/*
	** Enable the Data Ready Interrupt and route it to INT1.
	*/
	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG4, 0x84);  
	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG5, 0x84); 

#endif


#if TRANSIENT

///瞬态检测设置START
	IIC_RegWrite(MA8452Q_ADDR, TRANSIENT_CFG_REG, 0x1E);

	IIC_RegWrite(MA8452Q_ADDR, TRANSIENT_THS_REG, 0x10);
	//阈值寄存器0~127，阈值的最低分辨率为0.063g/LSB. 1.1g/0.063g=17.46. 四舍五入为18，阈值设置为12H

	IIC_RegWrite(MA8452Q_ADDR, TRANSIENT_COUNT_REG, 0x05); //100hz 50 ms debounce timing


///瞬态检测设置END


	/*
	** Configure the INT pins for Open Drain and Active Low
	*/
	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG3, 0X40);//0X00  PP_OD_MASK 注意：IPOL = 0 :该位代表中断脉冲当前是一直处于高电平，MCU端采用下降沿触发，否则失败
	/*
	** Enable the Data Ready Interrupt and route it to INT1.
	*/
	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG4, 0x20);  //04 24
	IIC_RegWrite(MA8452Q_ADDR,CTRL_REG5, 0x20); //24

#endif 	

	MMA845xActive(  );
}

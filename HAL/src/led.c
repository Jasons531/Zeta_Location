/*
**************************************************************************************************************
*	@file	LED.c
*	@author Jason_531@163.com
*	@version V0.0.1
*	@date    
*	@brief Led状态函数
***************************************************************************************************************
*/

#include <stdint.h>
#include "debug.h"
#include "led.h"

LedStates_t LedStates;

/*
*设置LED状态
*/
void SetLedStates(LedStates_t States)
{	
	LedStates = States;
	
	DEBUG_APP(3,"LedStates = %d",LedStates);
}

/*
*还原LED状态
*/
void RestLedStates(LedStates_t States)
{
	LedStates = States;
	
	DEBUG_APP(2,"RestLedStates = %d",LedStates);
}

/*
*获取LED状态
*/
LedStates_t GetLedStates(void)
{
	LedStates_t States;
	
	States = LedStates;
	
	return States;
}

/*
*LED初始化
*/
void LedInit(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOC_CLK_ENABLE(  );          

	GPIO_Initure.Pin=LED_RED|LED_GREEN;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  
	GPIO_Initure.Pull=GPIO_PULLUP;          
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     
	HAL_GPIO_Init(LED_PORT,&GPIO_Initure);
		
	LedOff(  );
}

/*
*按键开机指示灯
*/
void PowerOn(void)
{
	for(uint8_t i = 0; i < 10; ++i)
	{
		HAL_GPIO_TogglePin(LED_PORT,LED_GREEN);	
		HAL_Delay(400);
	}
	HAL_GPIO_WritePin(LED_PORT,LED_GREEN,GPIO_PIN_RESET);
}

/*
*按键关机指示灯
*/
void PowerOff(void)
{
	for(uint8_t i = 0; i < 10; ++i)
	{
		HAL_GPIO_TogglePin(LED_PORT,LED_RED);
		HAL_Delay(400);
	}
	HAL_GPIO_WritePin(LED_PORT,LED_RED,GPIO_PIN_RESET);
}

/*
*LED亮
*/
void LedOn(void)
{
	HAL_GPIO_WritePin(LED_PORT,LED_RED|LED_GREEN,GPIO_PIN_SET);
}

/*
*LED灭
*/
void LedOff(void)
{
	HAL_GPIO_WritePin(LED_PORT,LED_RED|LED_GREEN,GPIO_PIN_RESET);
}

/*
*LED翻转
*/
void LedToggle(void)
{
	HAL_GPIO_TogglePin(LED_PORT,LED_RED|LED_GREEN);
}

/*
*发送数据成功LED状态
*/
void LedSendSucess(int8_t Counter)
{
	if(GetLedStates(  ) == GpsLocation)  ///还原定位定时器
	HAL_TIM_Base_Stop_IT(&htim2);   ///定位过程关闭定时器，防止LED状态干扰
		
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedToggle(  );
		HAL_Delay(500);
	}
	LedOff(  );
	
	if(GetLedStates(  ) == GpsLocation)  ///还原定位定时器
	{
		HAL_TIM_Base_Start_IT(&htim2);
	}
}

/*
*发送数据失败LED状态
*/
void LedSendFail(int8_t Counter)
{
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedOn(  );
		HAL_Delay(1000);
		LedOff(  );
		HAL_Delay(200);
	}
}

/*
*接收数据LED状态
*/
void LedRev(int8_t Counter)
{
	if(GetLedStates(  ) == GpsLocation)  ///还原定位定时器
	HAL_TIM_Base_Stop_IT(&htim2);   ///定位过程关闭定时器，防止LED状态干扰
	
	for( int8_t i = Counter; i > 0; i -- )
	{
		LedToggle(  );
		HAL_Delay(200);
	}
	LedOff(  );
	
	if(GetLedStates(  ) == GpsLocation)  ///还原定位定时器
	{
		HAL_TIM_Base_Start_IT(&htim2);
		DEBUG_APP(3,"----Start_IT----");
	}
}

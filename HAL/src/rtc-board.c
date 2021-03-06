/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdbool.h>
#include "board.h"
#include "rtc-board.h"

RTC_HandleTypeDef RtcHandle;

RTC_TimeTypeDef  RTC_TimeStruct;
RTC_DateTypeDef  RTC_Datestructure;
RTC_AlarmTypeDef RTC_AlarmStruct;

/***********************************以下为RTC底层代码部分***************************************/
/* RTC init function */
void RTC_Init(void)
{				
	/**Initialize RTC Only 
	*/
	RtcHandle.Instance = RTC;
	RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
	RtcHandle.Init.AsynchPrediv = 36;
	RtcHandle.Init.SynchPrediv = 999;
	RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
	RtcHandle.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
		Error_Handler( );
	}
		/**Initialize RTC and set the Time and Date 
		*/
	RTC_TimeStruct.Hours = 0;
	RTC_TimeStruct.Minutes = 0;
	RTC_TimeStruct.Seconds = 0;
	RTC_TimeStruct.SubSeconds = 0;
	RTC_TimeStruct.TimeFormat = RTC_HOURFORMAT12_AM;    
	RTC_TimeStruct.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	RTC_TimeStruct.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler( );
	}

	RTC_Datestructure.WeekDay = RTC_WEEKDAY_MONDAY;
	RTC_Datestructure.Month = RTC_MONTH_JANUARY;
	RTC_Datestructure.Date = 1;
	RTC_Datestructure.Year = 19;

	if (HAL_RTC_SetDate(&RtcHandle, &RTC_Datestructure, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler( );
	}
	
	/*******************开启RTC中断*******************/
	HAL_NVIC_SetPriority(RTC_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(RTC_IRQn);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(RTC_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

/*
*RtcvRtcCalibrate: RTC内部时钟检验函数
*参数：						 无
*返回值：					 无
*/
void RtcvRtcCalibrate(void)
{
	uint32_t TickDelay = 0;
	uint32_t RtcSSR = 0;
	
	uint32_t CalibrateSynchPrediv = 0;
		
	/***************计算RTC定时1S，实际使用时间****************/
	TickDelay = HAL_GetTick();
  RtcSSR = RtcHandle.Instance->SSR;
	HAL_Delay(2);
	
	while(RtcSSR!=RtcHandle.Instance->SSR && HAL_GetTick()-TickDelay < 3000);
	if( HAL_GetTick()-TickDelay >= 3000 )
	{
			DEBUG_APP(3,"RTC校准失败2\r\n");
			return ;
	}
	TickDelay = HAL_GetTick()-TickDelay;
	DEBUG_APP(3,"ulActualDelay:%u\r\n",TickDelay);
	
	//if(ulActualDelay>=999 && ulActualDelay<=1001)
	if( TickDelay == 1000 )
	{
			DEBUG_APP(3,"无须校准\r\n");
			return ;
	}
	
	/* 根据实际延时计算新的分频值,四舍五入 */
	CalibrateSynchPrediv = (RtcHandle.Init.SynchPrediv+1) * 1000/TickDelay -0.5;
	DEBUG_APP(2, "CalibrateSynchPrediv:%u\r\n",CalibrateSynchPrediv);
	if( CalibrateSynchPrediv > INT_LEAST16_MAX )
	{
			DEBUG_APP(3,"RTC校准失败3\r\n");
			return ;
	}
	
	RtcHandle.Init.AsynchPrediv = 36;
  RtcHandle.Init.SynchPrediv = CalibrateSynchPrediv;
	
	RtcHandle.State = HAL_RTC_STATE_RESET;
	
	if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
	{
			DEBUG_APP(3,"RTC校准失败4\r\n");
	}
	DEBUG_APP(2, "RTC频率:%uHz\r\n",(RtcHandle.Init.AsynchPrediv+1)*(RtcHandle.Init.SynchPrediv+1));
}

/*******************************以上为RTC底层代码部分****************************************/

/*GetCurrentSleepRtc：意外唤醒触发，重新获取休眠时间
*参数								：无
*返回值							：无
*注意								：HAL_RTC_GetTime必须与HAL_RTC_GetDate一起使用，否则RTC时间不更新
*/
uint32_t GetCurrentSleepRtc(void)
{
	uint32_t CurrentRtc = 0;
	
	uint32_t AlarmTime = 0;
	
	//关闭RTC相关中断，可能在RTC实验打开了
	__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RtcHandle,RTC_IT_WUT);
	__HAL_RTC_TIMESTAMP_DISABLE_IT(&RtcHandle,RTC_IT_TS);
	__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle,RTC_IT_ALRA|RTC_IT_ALRB);
	
	//清除RTC相关中断标志位
	__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
	__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RtcHandle,RTC_FLAG_TSF); 
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle,RTC_FLAG_WUTF);
	
	/* 清除所有唤醒标志位 */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	
	HAL_RTC_WaitForSynchro(&RtcHandle);
	
	HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
	HAL_RTCEx_DeactivateWakeUpTimer( &RtcHandle );
		
	HAL_NVIC_DisableIRQ(RTC_IRQn);
		
	HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
	
	HAL_RTC_GetDate(&RtcHandle, &RTC_Datestructure, RTC_FORMAT_BIN);
	
	HAL_RTC_GetAlarm(&RtcHandle, &RTC_AlarmStruct, RTC_ALARM_A, RTC_FORMAT_BIN);
		
	CurrentRtc 			= RTC_TimeStruct.Hours * 3600 + RTC_TimeStruct.Minutes * 60 + RTC_TimeStruct.Seconds;
	
	AlarmTime  			= RTC_AlarmStruct.AlarmTime.Hours * 3600 + RTC_AlarmStruct.AlarmTime.Minutes * 60 + RTC_AlarmStruct.AlarmTime.Seconds;
	
	User.CurrentDate	= RTC_Datestructure.Date;
	
	User.AlarmDate		= RTC_AlarmStruct.AlarmDateWeekDay;
	
	DEBUG_APP(2,"GetcurrentDate:  %d  hour : %d min : %d second : %d",RTC_Datestructure.Date, RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
	
	DEBUG_APP(2,"GetAlarmDate:    %d   hour : %d min : %d second : %d",RTC_AlarmStruct.AlarmDateWeekDay, RTC_AlarmStruct.AlarmTime.Hours,RTC_AlarmStruct.AlarmTime.Minutes,RTC_AlarmStruct.AlarmTime.Seconds);

	HAL_NVIC_EnableIRQ(RTC_IRQn);
	
	return 	(AlarmTime > CurrentRtc)?(AlarmTime - CurrentRtc):0;
}

/*SetRtcAlarm：设置RTC闹钟休眠唤醒
*/
void SetRtcAlarm(uint16_t time)
{
	RTC_AlarmTypeDef RTC_AlarmStructure;
	
	uint8_t AlarmDate = 0;
		
	HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
	HAL_RTCEx_DeactivateWakeUpTimer( &RtcHandle );
	
	HAL_NVIC_DisableIRQ(RTC_IRQn);
	
	///休眠前校准RTC时钟
  RtcvRtcCalibrate(  );
	
	HAL_RTC_WaitForSynchro(&RtcHandle);
	
	HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle, &RTC_Datestructure, RTC_FORMAT_BIN);
	
	DEBUG_APP(2,"currentday: %d hour : %d min : %d second : %d",RTC_Datestructure.Date, RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);

	RTC_AlarmStructure.AlarmTime.Seconds		= (RTC_TimeStruct.Seconds+time)%60;  
	RTC_AlarmStructure.AlarmTime.Minutes 		= (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)%60;
	RTC_AlarmStructure.AlarmTime.Hours 	 		= (RTC_TimeStruct.Hours + (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)/60)%24;
		
	RTC_AlarmStructure.AlarmTime.SubSeconds = 0;
	
	AlarmDate = ((RTC_TimeStruct.Hours + (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)/60)/24 \
							+ RTC_Datestructure.Date);
							
	RTC_AlarmStructure.AlarmDateWeekDay			= ((AlarmDate > 31)? AlarmDate%31:AlarmDate); //按date匹配
	RTC_AlarmStructure.AlarmDateWeekDaySel 	= RTC_ALARMDATEWEEKDAYSEL_DATE; ///按date匹配
	RTC_AlarmStructure.AlarmMask 						= RTC_ALARMMASK_NONE;///精准匹配到date：全部去除就是精准匹配，开启某位则去除匹配
	RTC_AlarmStructure.AlarmSubSecondMask 	= RTC_ALARMSUBSECONDMASK_NONE;
	RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	RTC_AlarmStructure.Alarm 								= RTC_ALARM_A;
	
	DEBUG_APP(2,"wakeupday : %d  hour : %d min : %d second : %d",RTC_AlarmStructure.AlarmDateWeekDay, RTC_AlarmStructure.AlarmTime.Hours,RTC_AlarmStructure.AlarmTime.Minutes,RTC_AlarmStructure.AlarmTime.Seconds);
    	
	if( HAL_RTC_SetAlarm_IT( &RtcHandle, &RTC_AlarmStructure, RTC_FORMAT_BIN ) != HAL_OK )
	{
		assert_param( FAIL );
	}
    
  HAL_NVIC_EnableIRQ(RTC_IRQn);
}

/*ResetRtcAlarm：设置RTC闹钟休眠唤醒
*/
void ResetRtcAlarm(uint8_t date, uint16_t time)
{
	RTC_AlarmTypeDef RTC_AlarmStructure;
	
	uint8_t AlarmDate = 0;
		
	HAL_RTC_DeactivateAlarm( &RtcHandle, RTC_ALARM_A );
	HAL_RTCEx_DeactivateWakeUpTimer( &RtcHandle );
	
	HAL_NVIC_DisableIRQ(RTC_IRQn);
	
	///休眠前校准RTC时钟
  RtcvRtcCalibrate(  );
	
	HAL_RTC_WaitForSynchro(&RtcHandle);
	
	HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&RtcHandle, &RTC_Datestructure, RTC_FORMAT_BIN);
	
	DEBUG_APP(2,"currentday: %d hour : %d min : %d second : %d",RTC_Datestructure.Date, RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);

	RTC_AlarmStructure.AlarmTime.Seconds		= (RTC_TimeStruct.Seconds+time)%60;  
	RTC_AlarmStructure.AlarmTime.Minutes 		= (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)%60;
	RTC_AlarmStructure.AlarmTime.Hours 	 		= (RTC_TimeStruct.Hours + (RTC_TimeStruct.Minutes + (RTC_TimeStruct.Seconds+time)/60)/60)%24;
		
	RTC_AlarmStructure.AlarmTime.SubSeconds = 0;
								
	RTC_AlarmStructure.AlarmDateWeekDay			= date; //按date匹配
	RTC_AlarmStructure.AlarmDateWeekDaySel 	= RTC_ALARMDATEWEEKDAYSEL_DATE; ///按date匹配
	RTC_AlarmStructure.AlarmMask 						= RTC_ALARMMASK_NONE;///精准匹配到date：全部去除就是精准匹配，开启某位则去除匹配
	RTC_AlarmStructure.AlarmSubSecondMask 	= RTC_ALARMSUBSECONDMASK_NONE;
	RTC_AlarmStructure.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
	RTC_AlarmStructure.Alarm 								= RTC_ALARM_A;
	
	DEBUG_APP(2,"wakeupday : %d  hour : %d min : %d second : %d",RTC_AlarmStructure.AlarmDateWeekDay, RTC_AlarmStructure.AlarmTime.Hours,RTC_AlarmStructure.AlarmTime.Minutes,RTC_AlarmStructure.AlarmTime.Seconds);
    	
	if( HAL_RTC_SetAlarm_IT( &RtcHandle, &RTC_AlarmStructure, RTC_FORMAT_BIN ) != HAL_OK )
	{
		assert_param( FAIL );
	}
    
  HAL_NVIC_EnableIRQ(RTC_IRQn);
}



/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

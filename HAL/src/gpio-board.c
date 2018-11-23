/*
**************************************************************************************************************
*	@file	gpio-board.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#include "board.h"
#include "gpio-board.h"

void EXTI0_1_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );

}

void EXTI2_3_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}


void EXTI4_15_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );

}

/**
  * ��������: ���ڼ�ⰴ���Ƿ񱻳�ʱ�䰴��
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ����1 ����������ʱ�䰴��  0 ������û�б���ʱ�䰴��
  */
uint8_t CheckPowerkey(void)
{			
	uint8_t downCnt =0;																//��¼���µĴ���
	uint8_t upCnt =0;																//��¼�ɿ��Ĵ���			
 
	while(1)																		//��ѭ������return����
	{	
		/*************** 1S������ʱ ***************/
		HAL_Delay(10);																//�ӳ�һ��ʱ���ټ��
		if(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON)			//��⵽���°���
		{		
			downCnt++;																//��¼���´���
			upCnt=0;																//��������ͷż�¼
			if(downCnt>=100)														//����ʱ���㹻
			{
				DEBUG(2,"������Դ��ť \r\n");
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///�ȴ������ͷ�
				return 1; 															//��⵽������ʱ�䳤����
			}
		}
		else 
		{
			upCnt++; 																//��¼�ͷŴ���
			if(upCnt>5)																//������⵽�ͷų���5��
			{
				DEBUG(2,"����ʱ�䲻��\r\n");		
				while(HAL_GPIO_ReadPin(POWER_KEY_IO,POWER_KEY) == KEY_ON); ///�ȴ������ͷ�
				return 0;															//����ʱ��̫�̣����ǰ�����������
			}
		}
	}
}

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	�жϻص������������ж��¼�----����IO�жϣ�������Ӧ��DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	switch(GPIO_Pin)
	{
		case GPIO_PIN_0:  ///POWER_KEY
			if(CheckPowerkey(		))
			{
				DEBUG(2,"�ػ� \r\n");
				BoardEnterStandby(	);
			}
			else
			{
				DEBUG(2,"�����ж�\r\n");
			}		

		break;
		
		case GPIO_PIN_1:	///Zeta_Rev_Key
			
			if(HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN)) ///��ֹ�ж��󴥷�
			{
				DEBUG_APP(2,"PIN = %d",HAL_GPIO_ReadPin(ZETAINT_IO,ZETAINT_PIN));

				ZetaHandle.Interrupt(  );	
											
				UserDownCommand(  );
			}
		
		break;
		
		case GPIO_PIN_8:
			
		if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8))
		{
			DEBUG_APP(2,);
			MMA8452MultipleRead(  );
		}

		break;
		
		case GPIO_PIN_13:	
		if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13))
		{
			DEBUG_APP(2,);
		}
		
		break;
		
		case GPIO_PIN_15:
			DEBUG_APP(2,);

		break;
		
		default	:
			break;
	}
}

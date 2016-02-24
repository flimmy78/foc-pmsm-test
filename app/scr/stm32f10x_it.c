/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * *@brife�����ļ��Ĺ��ܰ���3���ж�

		1.ADC1_2_IRQHandler:��ɴ�Start��RUN���л������FOC���㣬��Ӧģ�⿴�Ź��ж�
		ģ�⿴�Ź��жϿ�����������ɲ���ź�
		
		2.TIM1_BRK_IRQHandler����Ӧ�����ж�
		
		3.TIM1_UP_IRQHandler����ӦTIM1����������жϣ�ʹ��ADC�ⲿת����������ADC�ж�
		FOC����ʱ�ر���ADC����ת������ÿ�����������ʱ��������
		
		PWM_FRQ=20Khz,���Ķ���ģʽ��Period=1800,��ô������������1/(2*20K)=25us
		FOC����ʱ��ʱ���Ƕ���أ�������˵һ��FOC����һ��PWM���ڣ�����FOC���㣬һ��PWM����
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "stm32f10x_MClib.h"
#include "MC_Globals.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

extern unsigned char Res_f;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BRAKE_HYSTERESIS (u16)((OVERVOLTAGE_THRESHOLD/16)*15)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/
void ADC1_2_IRQHandler(void)
{	
	/*1.ע��ת���ж�*/
	if((ADC1->SR & ADC_FLAG_JEOC) == ADC_FLAG_JEOC)
	{
		//It clear JEOC flag
		ADC1->SR = ~(u32)ADC_FLAG_JEOC;

		if (SVPWMEOCEvent())
		{
			MCL_Calc_BusVolt( );
			switch (State)
			{
				case RUN:          
					FOC_Model( );       
				break;       

				case START:        
					ENC_Start_Up( );					
				break; 

				default:
				break;
			}			
		}
	}

	/*2.ģ�⿴�Ź��ж�*/
	else 
	{
		if(ADC_GetITStatus(ADC1, ADC_IT_AWD) == SET)
		{	
			if(MCL_Chk_BusVolt( )==OVER_VOLT)  // ��ֹ����
			MCL_SetFault(OVER_VOLTAGE);
			ADC_ClearFlag(ADC1, ADC_FLAG_AWD);
		}    
	}
}
/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
  if(Res_f==1)//�ϵ���ɲ��ҽ��뵽ɲ���ж�
  MCL_SetFault(OVER_CURRENT);
  TIM_ClearITPendingBit(TIM1, TIM_IT_Break);
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
	// Clear Update Flag
	TIM_ClearFlag(TIM1, TIM_FLAG_Update); 
	SVPWMUpdateEvent( );
}
/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/




/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

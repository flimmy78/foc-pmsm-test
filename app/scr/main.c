/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : main.c
* Author             : IMS Systems Lab
* Date First Issued  : 21/11/07
* Description        : Main program body.
********************************************************************************
* History:
* 21/11/07 v1.0
* 29/05/08 v2.0
* 27/06/08 v2.0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_conf.h"
#include "stm32f10x_MClib.h"
#include "MC_Globals.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/


void RCC_Configuration(void);
unsigned char Res_f=0;	 //�ϵ��ֹ���ֹ���

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{			
	SystemInit();
	
	/*�ٶȷ�����ʼ��*/	
	ENC_Init( );
	
	/*����������ʼ��*/
	SVPWM_3ShuntInit();
	
	/*ʱ����ʼ����������TIM6�����ٶȣ���ʼ��PID*/
	TB_Init( );
	Tim6_Init( );
	PID_Init(&PID_Torque_InitStructure, &PID_Flux_InitStructure, &PID_Speed_InitStructure);	
	
	/*�¶ȣ���ѹ�����ʼ��*/
	MCL_Init_Arrays();  

	/*���������ʼ��*/		
	KEYS_Init( );
	LCD_Display_init();
	/*-------------------*/
	Res_f=1; //�ϵ����

	//����ʾ������ʼ��
	usart_init(115200);
	while(1)
	{ 
		
		/*UI��ʾ���Լ���Դ�������û�����*/
		Display_LCD( );
		MCL_ChkPowerStage( );    
		KEYS_process( );

		/*״̬����������*/
		switch (State)
		{
			case IDLE:    //ͨ��sel��������INIT ����WAIT��FAULT�н���IDEL
			break;

			case INIT:
				MCL_Init( );//��ʼ��������Ʋ�
				TB_Set_StartUp_Timeout(3000);
				State = START; 
			break;

			case START:  
				//passage to state RUN is performed by startup functions; 
			break;

			case RUN:  //������й����У�����ٶȷ����Ƿ��������      
				if(ENC_ErrorOnFeedback() == TRUE)
				{
					MCL_SetFault(SPEED_FEEDBACK);
				}

			break;  

			case STOP:  //�ر�TIM1�������״̬תΪ�ȴ���ֹͣ������⣬����Valpha��VbetaΪ0����������ռ�ձ� 
				TIM_CtrlPWMOutputs(TIM1, DISABLE);
				State = WAIT;								        
				SVPWM_3ShuntAdvCurrentReading(DISABLE);											
				Stat_Volt_alfa_beta.qV_Component1 = Stat_Volt_alfa_beta.qV_Component2 = 0;
				SVPWM_3ShuntCalcDutyCycles(Stat_Volt_alfa_beta);
				TB_Set_Delay_500us(2000); // 1 sec delay		
			break;

			case WAIT:    // �ȴ��ٶ�Ϊ��ʱ����״̬תΪIDEL
				if (TB_Delay_IsElapsed( ) == TRUE) 
				{
					if(ENC_Get_Mechanical_Speed( ) ==0)             
					{              
						State = IDLE;              
					}
				}
			break;

			case FAULT: //״̬��ΪIDEL��ȫ�ֱ�����Ϊ��һ������                  
				if (MCL_ClearFault( ) == TRUE)
				{
					if(wGlobal_Flags & SPEED_CONTROL == SPEED_CONTROL)
					{
						//�ٶȿ���ģʽ
						bMenu_index = CONTROL_MODE_MENU_1;
					}
					else
					{
						//���ؿ���ģʽ
						bMenu_index = CONTROL_MODE_MENU_6;
					} 					
					State = IDLE;
					wGlobal_Flags |= FIRST_START;
					Hall_Start_flag=0;
				}
			break;

			default:        
			break;
		}	
		usart_watcher(Speed_Iq_Id_watch);
		/********End of Usart_watch**************/
	}
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

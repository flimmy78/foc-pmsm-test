/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : MC_FOC_Drive.c
* Author             : IMS Systems Lab 
* Date First Issued  : 21/11/07
* Description        : This file provides all the PMSM FOC drive functions.
* 
********************************************************************************
* History:
* 21/11/07 v1.0
* 29/05/08 v2.0
* 14/07/08 v2.0.1
* 28/08/08 v2.0.2
* 04/09/08 v2.0.3
********************************************************************************
*@brife ���ļ���Ҫ�����FOC������ʱ�򣬿�����ϸ߼���������Ż�FOC���㡣
		1.FOC_Model���������FOC����ʱ��
		��ȡIa��Ib����ֵ��Clarke�任�õ�Iapha��Ibetaֵ��Park�仯�õ�Iq��Idֵ��
		�Ͳο�ֵ�ȽϽ���PID����õ�Vq��Vdֵ��Vq��Vdֵ�����޷����㣬�õ�Vq*��Vd*��
		��Park�任�õ�Valpha��Vbeta����󾭹�SVPWM����õ��������ռ�ձȺ�ADC����
		ת���㣬�ȴ���һ�θ����¼�������
		2.ͨ���ٶȼ������زο������߸������غʹ�ͨ�Ĳο�������������ʵ��
		3.�Ժ�ͨ��λ�ü����ٶȲο�Ҳ������ʵ�֡�
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_conf.h"
#include "stm32f10x_MClib.h"
#include "MC_Globals.h"
#include "MC_const.h"
#include "MC_FOC_Drive.h"
#include "MC_PMSM_motor_param.h"


#define SATURATION_TO_S16(a)    if (a > S16_MAX)              \
                                {                             \
                                  a = S16_MAX;                \
                                }                             \
                                else if (a < -S16_MAX)        \
                                {                             \
                                  a = -S16_MAX;               \
                                }                             \
/* Private functions ---------------------------------------------------------*/
/* Private variable ----------------------------------------------------------*/
static volatile Curr_Components Stat_Curr_q_d_ref;
static Curr_Components Stat_Curr_q_d_ref_ref;
								

/*******************************************************************************
* Function Name : FOC_Init
* Description   : The purpose of this function is to initialize to proper values
*                 all the variables related to the field-oriented control
*                 algorithm. To be called once prior to every motor startup.
* Input         : None.
* Output        : None.
* Return        : None.
*******************************************************************************/
void FOC_Init (void)
{
  Stat_Curr_q_d_ref_ref.qI_Component1 = 0;
  Stat_Curr_q_d_ref_ref.qI_Component2 = 0;  
  
  Stat_Curr_q_d_ref.qI_Component1 = 0;
  Stat_Curr_q_d_ref.qI_Component2 = 0;
}

/*******************************************************************************
* Function Name : FOC_Model
* Description   : The purpose of this function is to perform PMSM torque and 
*                 flux regulation, implementing the FOC vector algorithm.
* Input         : None.
* Output        : None.
* Return        : None.
*******************************************************************************/
void FOC_Model(void)
{	
  Stat_Curr_a_b =SVPWM_3ShuntGetPhaseCurrentValues();
  
  Stat_Curr_alfa_beta = Clarke(Stat_Curr_a_b);
  
  Stat_Curr_q_d = Park(Stat_Curr_alfa_beta,GET_ELECTRICAL_ANGLE);  

  /*loads the Torque Regulator output reference voltage Vqs*/   
  Stat_Volt_q_d.qV_Component1 = PID_Regulator(Stat_Curr_q_d_ref_ref.qI_Component1, 
                        Stat_Curr_q_d.qI_Component1, &PID_Torque_InitStructure);
  /*loads the Flux Regulator output reference voltage Vds*/
  Stat_Volt_q_d.qV_Component2 = PID_Regulator(Stat_Curr_q_d_ref_ref.qI_Component2, 
                          Stat_Curr_q_d.qI_Component2, &PID_Flux_InitStructure);  

  RevPark_Circle_Limitation( );
 
  Stat_Volt_alfa_beta = Rev_Park(Stat_Volt_q_d);

  /*Valpha and Vbeta finally drive the power stage*/ 
  SVPWM_3ShuntCalcDutyCycles(Stat_Volt_alfa_beta); 
}

/*******************************************************************************
* Function Name   : FOC_CalcFluxTorqueRef
* Description     : This function provides current components Iqs* and Ids* to be
*                   used as reference values (by the FOC_Model function) when in
*                   speed control mode
* Input           : None.
* Output          : None.
* Return          : None.
*******************************************************************************/
void FOC_CalcFluxTorqueRef(void)
{
	//PID �ٶȵ��ڵ�λ��RPM
	Stat_Curr_q_d_ref.qI_Component1 = PID_Regulator(hSpeed_Reference,GET_SPEED_0_1HZ*6,&PID_Speed_InitStructure);
	Stat_Curr_q_d_ref.qI_Component2 = 0;

	//�˴���Stat_Curr_q_d_ref_ref�Ǿ����㷨����֮��ĵ����ο�ֵ��֮�����˴�����㷨
	//Ȼ��Stat_Curr_q_d_ref ��ֱ�Ӿ����ٶ�PID����֮��ĵ����ο�ֵ
	Stat_Curr_q_d_ref_ref = Stat_Curr_q_d_ref;

	hTorque_Reference = Stat_Curr_q_d_ref_ref.qI_Component1;
	hFlux_Reference = Stat_Curr_q_d_ref_ref.qI_Component2;  
}


/*******************************************************************************
* Function Name   : FOC_TorqueCntrl
* Description     : This function provides current components Iqs* and Ids* to be
*                   used as reference values (by the FOC_Model function) when in
*                   Torque control mode
* Input           : None.
* Output          : None.
* Return          : None.
*******************************************************************************/
void FOC_TorqueCtrl(void)
{
	Stat_Curr_q_d_ref_ref.qI_Component1 = hTorque_Reference;
	Stat_Curr_q_d_ref_ref.qI_Component2 = hFlux_Reference;
}




/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

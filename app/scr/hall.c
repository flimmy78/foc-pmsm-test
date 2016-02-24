/********************************************************************************
*@brife ���ļ���Ҫ������HALL��������ʹ�ã�����hall����ʹ�ã���û�в��Գɹ���

	
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_conf.h"
#include "hall.h"
#include "MC_Globals.h"
#include "stm32f10x_MClib.h"
#include "stm32f10x_it.h"

const unsigned char Clockwise_STEP_TAB[6]={4,0,5,2,3,1};//˳ʱ��ʱ��
const unsigned char Inv_Clockwise_STEP_TAB[6]={1,3,2,5,0,4};//��ʱ��ʱ��

const unsigned int PWM_EN_TAB[6]={0x055,0x505,0x550,0x055,0x505,0x550};
//	6��PWM���ʹ�ܿ���	AB,AC,BC,BA,CA,CB


/* Private define ------------------------------------------------------------*/
#define HALL_COUNTER_RESET  ((u16) 0)
#define S16_PHASE_SHIFT     HALL_PHASE_SHIFT
#define S16_120_PHASE_SHIFT 120
#define S16_60_PHASE_SHIFT  60

#define STATE_0 (u16)0
#define STATE_1 (u16)1
#define STATE_2 (u16)2
#define STATE_3 (u16)3
#define STATE_4 (u16)4
#define STATE_5 (u16)5
#define STATE_6 (u16)6
#define STATE_7 (u16)7

#define NEGATIVE          (s8)-1
#define POSITIVE          (s8)1


#define HALL_GPIO_MSK (u16)0x0007
#define HALL_ICx_FILTER (u8) 0x0B // 11 <-> 1333 nsec 

#define HALL_PRE_EMPTION_PRIORITY 2
#define HALL_SUB_PRIORITY 0

u8  ReadHallState(void); 
static s16 hElectrical_Angle; 

u16 PWM_duty = (u16)PWM_PERIOD*0.1;

u8 Hall_Start_flag	=	0;


s8 Read_dir(s16 speed_reference)
{
	s8 Dir;
	if(speed_reference >= 0)
		Dir=1;
	if (speed_reference < 0)
		Dir=-1;	
	return (Dir);
}

void delay_ms(int delay_ms)
{
	int i,j;
	for(i=delay_ms;i>0;i--)
	{
		j=72000;//����һ��ʱ����һ�μӷ�����ô72000����1ms���õ��Բ�ʱ��
		while(j>0)
			j--;
	}
}

void Halltimer_init(void)
{
	TIM_TimeBaseInitTypeDef  	TIM_TimeBaseStructure;
	TIM_ICInitTypeDef 			TIM_ICInitStructure;
	GPIO_InitTypeDef            GPIO_InitStructure;	
	NVIC_InitTypeDef            NVIC_InitStructure;
	 

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE); 

	/*	HALL: PA2=C	PA1=B  PA0=A	*/
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2 | GPIO_Pin_1 | GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);			   			   

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseStructure.TIM_Period 			= 0xffff;	// 65536*2us
	TIM_TimeBaseStructure.TIM_Prescaler 		= 72*2-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //�˲���ʱ����PSC��ϵ
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5,DISABLE);//��������װ��

	/* HALL ���벶�� 0����:channel 1 ,TRC */
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_TRC; //
	TIM_ICInitStructure.TIM_ICFilter 	= 5;
	TIM_ICInit(TIM5,&TIM_ICInitStructure);

	//Enables the XOR of channel 1, channel2 and channel3	 ��
	TIM_SelectHallSensor(TIM5, ENABLE);
	TIM_SelectInputTrigger(TIM5, TIM_TS_TI1F_ED);	//���ؼ�ⴥ��ѡ��

	TIM_Cmd(TIM5, ENABLE);
	TIM_ClearFlag(TIM5,TIM_FLAG_CC1);
	TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);			// ����HALL�ź�
	TIM_SelectSlaveMode(TIM5, TIM_SlaveMode_Reset);  

	NVIC_InitStructure.NVIC_IRQChannel =  TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = HALL_PRE_EMPTION_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = HALL_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}

u8 ReadHallState(void)
{
  u16 ReadValue;
  ReadValue= (u16)(GPIO_ReadInputData(GPIOA));
  ReadValue = ReadValue & HALL_GPIO_MSK;  
  return(ReadValue);
}

void BLDC_Change_phase( s8 Dir,u16 duty)
{
	u8 hallstate=0;
	u8 step	=	0;

	TIM_CtrlPWMOutputs(TIM1, DISABLE);
	hallstate=ReadHallState( );

	if(Dir>=0)//˳ʱ��ʱ��
		step=Clockwise_STEP_TAB[hallstate-1];
	else//��ʱ��ʱ��
		step=Inv_Clockwise_STEP_TAB[hallstate-1];	
	TIM1->CCER=PWM_EN_TAB[step];
	
	if(step<=1)	
	{
		TIM1->CCR1=PWM_duty;	
		TIM1->CCR2=0;
		TIM1->CCR3=0;
	}
	else if(step<=3)
	{
		TIM1->CCR1=0;
		TIM1->CCR2=PWM_duty;
		TIM1->CCR3=0;	
	}		
	else if (step<=5)
	{
		TIM1->CCR1=0;
		TIM1->CCR2=0;
		TIM1->CCR3=PWM_duty;
	}			
	else 
	{
		TIM1->CCR1=0;
		TIM1->CCR2=0;
		TIM1->CCR3=0;
	}
	
	TIM_CtrlPWMOutputs(TIM1, ENABLE);//ʹ�����PWM	 
}


u8 hall_state=0;
void HALL_Init_Electrical_Angle(void)
{   
	hall_state=ReadHallState();
	 switch(hall_state)
	 {
		case STATE_5:
			hElectrical_Angle = (s16)(S16_PHASE_SHIFT+S16_60_PHASE_SHIFT/2);
			break;
		case STATE_1:
			hElectrical_Angle =(s16)(S16_PHASE_SHIFT+S16_60_PHASE_SHIFT+S16_60_PHASE_SHIFT/2);
			break;
		case STATE_3:
			hElectrical_Angle =(s16)(S16_PHASE_SHIFT+S16_120_PHASE_SHIFT+S16_60_PHASE_SHIFT/2);      
			break;
		case STATE_2:
			hElectrical_Angle =(s16)(S16_PHASE_SHIFT-S16_120_PHASE_SHIFT-S16_60_PHASE_SHIFT/2);      
			break;
		case STATE_6:
			hElectrical_Angle =(s16)(S16_PHASE_SHIFT-S16_60_PHASE_SHIFT-S16_60_PHASE_SHIFT/2);          
			break;
		case STATE_4:
			hElectrical_Angle =(s16)(S16_PHASE_SHIFT-S16_60_PHASE_SHIFT/2);          
			break;    
		default:    
			break;
		}	
		
}

u16 wEncoder_reset=0;
u16 HALL_GetElectricalAngle(void)
{ 

	wEncoder_reset=(u16)((hElectrical_Angle*4*ENCODER_PPR/360)-1)/POLE_PAIR_NUM;
	return(wEncoder_reset);
}

void TIM5_IRQHandler(void)//������պ������λ�ã�ת�Ӳ��ᶯ���Ҳ�������жϣ���ó�ʼ��Ƕȡ����жϵ����ȼ�����Ϊ��߲��ɱ����
{
	if ( TIM_GetFlagStatus(HALL_TIMER, TIM_FLAG_CC1) == !RESET )
	{
		TIM_ClearFlag(HALL_TIMER, TIM_FLAG_CC1); 
		TIM_ClearITPendingBit(HALL_TIMER, TIM_IT_CC1);
		
		Hall_Start_flag ++;

		if (Hall_Start_flag==0) //ֻ�е�һ������������
		{		
			HALL_Init_Electrical_Angle( );
 			ENCODER_TIMER->CNT = HALL_GetElectricalAngle( );			
		}
		//һ����е����У��һ��
		if(Hall_Start_flag==POLE_PAIR_NUM*6)
			Hall_Start_flag=0;

	}
}


bool Hall_Startup(void)
{

	BLDC_Change_phase(Read_dir(hSpeed_Reference),PWM_duty);	
	
	Stat_Volt_q_d.qV_Component1 = Stat_Volt_q_d.qV_Component2 = 0;
	SVPWM_3ShuntCalcDutyCycles(Stat_Volt_alfa_beta);
	hTorque_Reference = PID_TORQUE_REFERENCE;
	hFlux_Reference = PID_FLUX_REFERENCE;
	wGlobal_Flags &= ~FIRST_START;   // alignment done only once 
	//Clear the speed acquisition of the alignment phase
	
	return(TRUE);	
}


//��һ�ζ�ȡhall��״̬���õ���һ�ε�Ƕ�
//BLDC_Change_phase(); ������ˣ�hall״̬�ı䣬���û�����ǶȲ���
//��ȡhall��״̬���õ��ڶ��ε�Ƕȡ�
//���ε�Ƕ���ͬ��ô������Ϊ��ʼֵ������ͬȡ����
//���������жϣ����������Ե�У����Ƕȡ�
//bool Hall_Startup(void)
//{
//	s16 angle_first=0;
//	s16 angle_second=0;

//	HALL_Init_Electrical_Angle();
//	angle_first = hElectrical_Angle;
//	
//	BLDC_Change_phase(Read_dir(hSpeed_Reference),PWM_duty);
//	
////	delay_ms(200);
//	
//	HALL_Init_Electrical_Angle();
//	angle_second = hElectrical_Angle;

//	TIM_ClearFlag(HALL_TIMER, TIM_FLAG_CC1); 
//	TIM_ClearITPendingBit(HALL_TIMER, TIM_IT_CC1);

//	if(angle_first!=angle_second)
//	angle_first=angle_second;

//	ENCODER_TIMER->CNT=(u16)(((angle_first*4*ENCODER_PPR/360)-1)/POLE_PAIR_NUM);	
//	
//	TIM_ITConfig(TIM5, TIM_IT_CC1, ENABLE);	
//	
//	return(TRUE);	
//}


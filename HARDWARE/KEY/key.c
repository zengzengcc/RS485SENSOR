#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "stmflash.h"
#include "usart.h"


void Timer3_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��
    
    //��ʱ��TIM3��ʼ��
    TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
    
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //ʹ��ָ����TIM3�ж�,��������ж�

    //�ж����ȼ�NVIC����
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

    TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx
}

//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��

	GPIO_InitStructure.GPIO_Pin  = KEY0_PIN|KEY1_PIN|KEY2_PIN|KEY3_PIN|KEY4_PIN|KEY5_PIN;//KEY0-KEY5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(KEY0_PORT, &GPIO_InitStructure);//��ʼ��

	
}

u8 GetKeyValue()
{
	static u8 inputdata;
	u8 i;
	for(i=4;i<10;i++)
	{
		if(PBin(i) == 0)
		{
			
			inputdata |=(0x1<<(i-4));
		}else
		{
			
			inputdata &= ~(0x1<<(i-4));
		}
	}
	return inputdata;
}

volatile u8  KeyCurrent,KeyOld,KeyNoChangedTime;
volatile u8  KeyPress;
volatile u8  KeyDown,KeyUp,KeyLast;

volatile u8 KeyCanChange;

void TIM3_IRQHandler(void)   //TIM3�ж�
{
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	
	//��ʼ����ɨ��
	//���水��״̬����ǰ�������
	//KeyCurrent�ܹ���8��bit
	//��ĳ�����ذ���ʱ����Ӧ��bitΪ1
	KeyCurrent=GetKeyValue(); //��ȡ��ֵ

	if(KeyCurrent!=KeyOld)  //�������ֵ���ȣ�˵��������������˸ı�
	{
		KeyNoChangedTime=0;       //���̰���ʱ��Ϊ0
		KeyOld=KeyCurrent;        //���浱ǰ�������
		return;  //����
	}
	else
	{
		KeyNoChangedTime++;	     //����ʱ���ۼ�
		if(KeyNoChangedTime>=1)	 //�������ʱ���㹻
		{
			KeyNoChangedTime=1;
			KeyPress=KeyOld;      //���水��
			KeyDown|=(~KeyLast)&(KeyPress); //����°��µļ�
			KeyUp|=KeyLast&(~KeyPress);     //������ͷŵļ�
			KeyLast=KeyPress;		         //���浱ǰ�������
		}
	}
	
}


//#define SIZE sizeof(save_data)		//���鳤��
//#define FLASH_SAVE_ADDR  0X08007000		//����FLASH �����ַ(����Ϊż��������ֵҪ���ڱ�������ռ��FLASH�Ĵ�С+0X08000000)
//static  u8 data_write_count = 0;
//static  u8 data_read_count = 0;
/*
u8 sys_mode = USUAL_MODE;
u16 datatemp[8] = {0};
void key_event_hadler()
{
	switch(KEY_RETURN)
	{
		case KEY0_PRES:	// ���� 
		{	
			STMFLASH_Write(FLASH_SAVE_ADDR + SIZE*data_write_count,save_data,SIZE);
			data_read_count = data_write_count;
			data_write_count++;
			if(data_write_count > 100) //���100������
			{
				data_write_count = 0;
			}
			break;
		}
		case KEY1_PRES: //��ȡ
		{
			sys_mode = !sys_mode;
			if(sys_mode == READ_MODE)
			{
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,(u16*)datatemp,SIZE);
				//������ʾ����Ļ
			}
			break;
		}
		case KEY2_PRES: //�����¶�����
		{
			if(sys_mode == READ_MODE)
			{
				data_read_count ++;
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,(u16*)datatemp,SIZE);
				//������ʾ����Ļ
			}else
			{
				
			}
			break;
		}
		case KEY3_PRES: //�����¶�����
		{
			break;
		}
		case KEY4_PRES: //����ʪ������
		{
			break;
		}
		case KEY5_PRES: //����ʪ������
		{
			break;
		}
	}
}
*/


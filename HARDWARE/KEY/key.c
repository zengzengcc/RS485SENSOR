#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "stmflash.h"
#include "usart.h"
#include "OLED.h"
#include "rtc.h"
#include "rs485.h"


volatile u8  KeyCurrent,KeyOld,KeyNoChangedTime;
volatile u8  KeyPress;
volatile u8  KeyDown,KeyUp,KeyLast;

volatile u8 KeyCanChange;

void Timer3_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
    
    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
    TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
    
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //使能指定的TIM3中断,允许更新中断

    //中断优先级NVIC设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

    TIM_Cmd(TIM3, ENABLE);  //使能TIMx
}

//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟

	GPIO_InitStructure.GPIO_Pin  = KEY0_PIN|KEY1_PIN|KEY2_PIN|KEY3_PIN|KEY4_PIN|KEY5_PIN;//KEY0-KEY5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(KEY0_PORT, &GPIO_InitStructure);//初始化

	KeyCanChange = 1;
}

static u8 two_second_begin = 0;
static u8 two_second_count = 0;
u8 callback_event = 0;

void key_event_callback(u8 key)
{

	switch (key)
	{
		case KEY0:
		{
			showCNString(84,6,"空空",FONT_16_EN);
			break;
		}
	}
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



void TIM3_IRQHandler(void)   //TIM3中断
{
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	
	if(!KeyCanChange)
		return;

	//开始键盘扫描
	//保存按键状态到当前按键情况
	//KeyCurrent总共有8个bit
	//当某个开关按下时，对应的bit为1
	KeyCurrent=GetKeyValue(); //读取键值

	if(KeyCurrent!=KeyOld)  //如果两次值不等，说明按键情况发生了改变
	{
		KeyNoChangedTime=0;       //键盘按下时间为0
		KeyOld=KeyCurrent;        //保存当前按键情况
		return;  //返回
	}
	else
	{
		KeyNoChangedTime++;	     //按下时间累计
		if(KeyNoChangedTime>=2)	 //如果按下时间足够
		{
			KeyNoChangedTime=1;
			KeyPress=KeyOld;      //保存按键
			KeyDown|=(~KeyLast)&(KeyPress); //求出新按下的键
			KeyUp|=KeyLast&(~KeyPress);     //求出新释放的键
			KeyLast=KeyPress;		         //保存当前按键情况
		}
	}
	key_event_hadler();			//按键处理函数
	if(two_second_begin == 1)
	{
		two_second_count++;
		if(two_second_count > 200)
		{
			two_second_begin = 0;
			two_second_count = 0;
			key_event_callback(callback_event);
		}
	}
}


#define SIZE sizeof(save_data)		//数组长度
#define FLASH_SAVE_ADDR  0X08007000		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)
static  u8 data_write_count = 0;
static  u8 data_read_count = 0;

u8 sys_mode = USUAL_MODE;
u16 datatemp[8] = {0};
void key_event_hadler()
{

	switch(KeyUp)
	{
		case KEY0:	// 保存 
		{	
			
			if(sys_mode == USUAL_MODE)
			{
				STMFLASH_Write(FLASH_SAVE_ADDR + SIZE*data_write_count,save_data,SIZE);
				data_read_count = data_write_count;
				data_write_count++;
				if(data_write_count > 100) //最大100组数据
				{
					data_write_count = 0;
				}
				//屏幕右下角显示“已保存”
				showCNString(84,6,"保存",FONT_16_EN);
				two_second_begin = 1;
				callback_event = KEY0;
			}
					
			break;
		}
		case KEY1: //读取
		{
			if(sys_mode != READ_MODE)
			{
				sys_mode = READ_MODE;
				data_read_count = data_write_count - 1;//读取位置等于当前写入位置-1
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，第一组数据显示在屏幕上
				formatScreen(0X00);
				showNumber(0,0,save_data[3],DEC,2,FONT_16_EN);		//显示时间 
				showNumber(24,0,save_data[4],DEC,2,FONT_16_EN);
				showNumber(48,0,save_data[5],DEC,2,FONT_16_EN);
			}else
			{
				sys_mode = USUAL_MODE;	//切换到非读取模式
			}

			break;
		}
		case KEY2: //调整温度上限  & 下一组数据
		{
			if(sys_mode == READ_MODE)
			{
				data_read_count ++;
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，数据显示在屏幕
				formatScreen(0X00);
				showNumber(0,0,save_data[3],DEC,2,FONT_16_EN);		//显示时间 
				showNumber(24,0,save_data[4],DEC,2,FONT_16_EN);
				showNumber(48,0,save_data[5],DEC,2,FONT_16_EN);
			}else
			{
				
				//显示“温度上限：XX°C”
				sensor.upper_temp_limit++;
				formatScreen(0X00);
				showCNString(0,3,"温度上限",FONT_16_CN);
				showNumber(68,3,sensor.upper_temp_limit,DEC,2,FONT_16_EN);
			}
			break;
		}
		case KEY3: //调整温度下限
		{
			if(sys_mode == READ_MODE)
			{
				data_read_count --;
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，数据显示在屏幕
				formatScreen(0X00);
				showNumber(0,0,save_data[3],DEC,2,FONT_16_EN);		//显示时间 
				showNumber(24,0,save_data[4],DEC,2,FONT_16_EN);
				showNumber(48,0,save_data[5],DEC,2,FONT_16_EN);
			}else
			{
				//显示“温度下限：XX°C”
				sensor.lower_temp_limit--;
				formatScreen(0X00);
				showCNString(0,3,"温度下限",FONT_16_CN);
				showNumber(68,3,sensor.lower_temp_limit,DEC,2,FONT_16_EN);
			}
			break;
		}
		case KEY4: //调整湿度上限
		{
			//显示“湿度上限：XX%”
			break;
		}
		case KEY5: //调整湿度下限
		{
			//显示“湿度下限：XX%”
			break;
		}
	}

	KeyUp = 0;
}



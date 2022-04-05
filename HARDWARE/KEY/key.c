#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "stmflash.h"
#include "usart.h"
#include "OLED.h"
#include "rtc.h"
#include "rs485.h"



u8 data_write_count = 0;
u8 data_read_count = 0;

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

static u16 five_second_begin = 0;
static u16 five_second_count = 0;

u8 callback_event = 0;

void key_event_callback(u8 key)
{

	switch (key)
	{
		case KEY0:
		{
			break;
		}
		case KEY1:
		{
			sys_mode = USUAL_MODE;
			break;
		}
		case KEY4:
		{
			sys_mode = USUAL_MODE;
			break;
		}
		default:
			break;
	}
	callback_event = 0;
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

	if(five_second_begin == 1)
	{
		five_second_count++;
		if(five_second_count > 500)
		{
			five_second_begin = 0;
			five_second_count = 0;
			key_event_callback(callback_event);
		}
	}
	


	
}
void show_savedata(void)
{
	formatScreen(0X00);
	showString(16,0,":",FONT_16_EN);
	showString(40,0,":",FONT_16_EN);
	showString(92,0,"-",FONT_16_EN);			
	showNumber(0,0,save_data[3],DEC,2,FONT_16_EN);		//显示时间 
	showNumber(24,0,save_data[4],DEC,2,FONT_16_EN);
	showNumber(48,0,save_data[5],DEC,2,FONT_16_EN);
	showNumber(76,0,save_data[1],DEC,2,FONT_16_EN);
	showNumber(100,0,save_data[2],DEC,2,FONT_16_EN);
	showCNString(0,3,"温度",FONT_16_CN);
	showString(33,3,":",FONT_16_EN);
	showCNString(0,6,"湿度",FONT_16_EN);
	showString(33,6,":",FONT_16_EN);
	showNumber(40,3,save_data[6]/10,DEC,2,FONT_16_EN);
	showString(56,3,".",FONT_16_EN);
	showNumber(60,3,save_data[6]%10,DEC,2,FONT_16_EN);
	showCNString(68,3,"镀",FONT_16_EN);		//°
	showNumber(40,6,save_data[7]/10,DEC,2,FONT_16_EN);
	showString(56,6,".",FONT_16_EN);
	showNumber(60,6,save_data[7]%10,DEC,2,FONT_16_EN);
	showString(68,6,"%",FONT_16_EN);

}




u8 sys_mode = USUAL_MODE;

void key_event_hadler()
{
	switch(KeyUp)
	{
		case KEY0:	// 保存 
		{	
			
			if(sys_mode == USUAL_MODE)
			{
				STMFLASH_Write(FLASH_SAVE_ADDR + SIZE*data_write_count,save_data,SIZE);
				data_write_count++;
				if(data_write_count > 100) //最大100组数据
				{
					data_write_count = 0;
				}
				//屏幕右下角显示“保存”
				showCNString(84,6,"保存",FONT_16_EN);
				two_second_begin = 1;
				callback_event = KEY0;
			}
					
			break;
		}
		case KEY1: //温度
		{
			if(sys_mode != ADJUST_TEMP_MODE)
			{
				sys_mode = ADJUST_TEMP_MODE;
				sensor.adjust_temp_limit = UP_LIMIT;
				
				
			}else
			{
				sensor.adjust_temp_limit = !sensor.adjust_temp_limit;
			}
			five_second_begin = 1;
			five_second_count = 0;
			callback_event = KEY1;
			break;
		}
		case KEY2: //+
		{
			if(sys_mode == READ_MODE)
			{
				if(data_read_count < (data_write_count-1))
				{
					data_read_count ++;
				}
			
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，数据显示在屏幕
				show_savedata();
			}
			if(sys_mode == ADJUST_TEMP_MODE)
			{
				five_second_count = 0;
				if(sensor.adjust_temp_limit == UP_LIMIT)
				{
					sensor.upper_temp_limit++;
				}
				if(sensor.adjust_temp_limit == LOW_LIMIT)
				{
					sensor.lower_temp_limit++;
				}
			}
			if(sys_mode == ADJUST_HUMI_MODE)
			{
				five_second_count = 0;
				if(sensor.adjust_humi_limit == UP_LIMIT)
				{
					sensor.upper_humi_limit++;
				}
				if(sensor.adjust_humi_limit == LOW_LIMIT)
				{
					sensor.lower_humi_limit++;
				}
			}
			break;
		}
		case KEY3: //读取
		{
			
			if(sys_mode != READ_MODE)
			{
				sys_mode = READ_MODE;
				if(data_write_count > 0)
				{
					data_read_count = data_write_count - 1;//读取位置等于当前写入位置-1	
				}

				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，第一组数据显示在屏幕上
				show_savedata();
			}else
			{
				sys_mode = USUAL_MODE;	//切换到非读取模式
			}

			break;
		}
		case KEY4: //湿度
		{
			if(sys_mode != ADJUST_HUMI_MODE)
			{
				sys_mode = ADJUST_HUMI_MODE;
				sensor.adjust_humi_limit = UP_LIMIT;
				
				
			}else
			{
				sensor.adjust_humi_limit = !sensor.adjust_humi_limit;
			}
			five_second_begin = 1;
			five_second_count = 0;
			callback_event = KEY4;
			break;
			
		}
		case KEY5: // -
		{
			if(sys_mode == READ_MODE)
			{
				if(data_read_count > 0)
				{
					data_read_count --;
				}
				STMFLASH_Read(FLASH_SAVE_ADDR + SIZE*data_read_count,save_data,SIZE);
				//清屏幕，数据显示在屏幕
				show_savedata();
			}
			if(sys_mode == ADJUST_TEMP_MODE)
			{
				five_second_count = 0;
				if(sensor.adjust_temp_limit == UP_LIMIT)
				{
					sensor.upper_temp_limit--;
				}
				if(sensor.adjust_temp_limit == LOW_LIMIT)
				{
					sensor.lower_temp_limit--;
				}
			}
			if(sys_mode == ADJUST_HUMI_MODE)
			{
				five_second_count = 0;
				if(sensor.adjust_humi_limit == UP_LIMIT)
				{
					sensor.upper_humi_limit--;
				}
				if(sensor.adjust_humi_limit == LOW_LIMIT)
				{
					sensor.lower_humi_limit--;
				}
			}
			break;
		}
	}

	KeyUp = 0;
}



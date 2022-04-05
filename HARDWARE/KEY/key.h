#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////   	 
#define SIZE sizeof(save_data)		//数组长度
#define FLASH_SAVE_ADDR  0X08007000		//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)

#define KEY0_PORT GPIOB
#define KEY1_PORT GPIOB
#define KEY2_PORT GPIOB
#define KEY3_PORT GPIOB
#define KEY4_PORT GPIOB
#define KEY5_PORT GPIOB

#define KEY0_PIN    GPIO_Pin_4  
#define KEY1_PIN    GPIO_Pin_5
#define KEY2_PIN    GPIO_Pin_6
#define KEY3_PIN    GPIO_Pin_7
#define KEY4_PIN    GPIO_Pin_8
#define KEY5_PIN    GPIO_Pin_9

#define KEY0 0x01
#define KEY1 0x02
#define KEY2 0x04
#define KEY3 0x08
#define KEY4 0x10
#define KEY5 0x20 


enum
{
    USUAL_MODE,
    READ_MODE,
    ADJUST_TEMP_MODE,
    ADJUST_HUMI_MODE,
};

enum
{
    UP_LIMIT = 0,
    LOW_LIMIT = 1,
};

extern volatile u8  KeyCurrent,KeyOld,KeyNoChangedTime;
extern volatile u8  KeyPress;
extern volatile u8  KeyDown,KeyUp,KeyLast;

extern volatile u8 KeyCanChange;


extern u16 save_data[8];
extern u8 sys_mode;
extern u8 data_write_count;
extern u8 data_read_count;

void KEY_Init(void);//IO初始化	
void Timer3_Init(u16 arr,u16 psc);
void key_event_hadler(void);
void show_savedata(void);
u8 GetKeyValue(void);
#endif

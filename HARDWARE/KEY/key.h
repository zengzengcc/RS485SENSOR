#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////   	 

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
};

extern volatile u8  KeyCurrent,KeyOld,KeyNoChangedTime;
extern volatile u8  KeyPress;
extern volatile u8  KeyDown,KeyUp,KeyLast;

extern volatile u8 KeyCanChange;


extern u16 save_data[8];

void KEY_Init(void);//IO≥ı ºªØ	
void Timer3_Init(u16 arr,u16 psc);
void key_event_hadler(void);
u8 GetKeyValue(void);
#endif

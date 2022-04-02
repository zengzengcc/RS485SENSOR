#ifndef __LED_H
#define __LED_H	 
#include "sys.h"



//////////////////////////////////////////////////////////////////////////////////

#define LED0_PORT GPIOB
#define LED1_PORT GPIOB
#define LED2_PORT GPIOB
#define LED3_PORT GPIOB
#define BEEP_PORT GPIOB

#define LED0_PIN GPIO_Pin_12
#define LED1_PIN GPIO_Pin_13
#define LED2_PIN GPIO_Pin_14
#define LED3_PIN GPIO_Pin_15
#define BEEP_PIN GPIO_Pin_3

#define LED_B PCout(13)// PC13
#define LED0  PBout(12) //PB12
#define LED1  PBout(13) //PB13
#define LED2  PBout(14) //PB14
#define LED3  PBout(15) //PB15

#define BEEP PBout(3) //PB3 

void LED_Init(void);//初始化

		 				    
#endif

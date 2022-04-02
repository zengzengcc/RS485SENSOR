#include "led.h"

 

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PC、PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		//推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 	//IO口速度为2MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);								//根据设定参数初始化GPIOC.13
	
 GPIO_InitStructure.GPIO_Pin = BEEP_PIN|LED0_PIN|LED1_PIN|LED2_PIN|LED3_PIN;				 
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为2MHz
 GPIO_Init(LED0_PORT, &GPIO_InitStructure);					 				//根据设定参数初始化GPIOB
}
 

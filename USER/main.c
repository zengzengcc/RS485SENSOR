#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "rs485.h"	 
#include "IIC.h"
#include "OLED.h"
#include "rtc.h"
#include "stmflash.h"
#include <stdlib.h>
#include <string.h>			

u8 tempbuf[] = {0x01,0x04,0x00,0x01,0x00,0x01,0x60,0x0a}; // 读取温度
u8 humibuf[] = {0x01,0x04,0x00,0x02,0x00,0x01,0x90,0x0A}; //读取湿度

u16 save_data[8] = {0};				// year、month、day、hour、min，sec，temp、humi


int main(void)
 {	 
	u8 rs485_rxbuf[10] = {0};
	char temp_data[5];
	char humi_data[5];
	u8 *len;

	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	LED_Init();		  		//初始化与LED连接的硬件接口	
	KEY_Init();				//按键初始化		 	 
	RS485_Init(9600);		//初始化RS485		
	initIIC();				//OLED IIC接口初始化
	RTC_Init();	  			//RTC初始化
 	initOLED();
	Timer3_Init(99,7199); //10ms
	formatScreen(0x00);
   showCNString(0,3,"温度",FONT_16_CN);
   showString(33,3,":",FONT_16_EN);
   showCNString(0,6,"湿度",FONT_16_EN);
   showString(33,6,":",FONT_16_EN);
 	
	showString(16,0,":",FONT_16_EN);
	showString(40,0,":",FONT_16_EN);
	showString(92,0,"-",FONT_16_EN);
	
	showString(76,3,"C",FONT_16_EN);
	showString(56,3,".",FONT_16_EN);
	showString(68,6,"%",FONT_16_EN);
	showString(56,6,".",FONT_16_EN);
	showCNString(68,3,"镀",FONT_16_EN);		//°
	while(1)
	{
	
		RS485_Send_Data(tempbuf,8);// 获取温度
		delay_ms(200);		
		RS485_Receive_Data(rs485_rxbuf,len);
		sprintf(temp_data,"%d",rs485_rxbuf[3]<<8|rs485_rxbuf[4]);
		save_data[6] = atoi(temp_data);

		showNumber(40,3,atoi(temp_data)/10,DEC,2,FONT_16_EN);
		showString(56,3,".",FONT_16_EN);
		showNumber(60,3,atoi(temp_data)%10,DEC,2,FONT_16_EN);
		
		
		RS485_Send_Data(humibuf,8);// 获取湿度
		delay_ms(200);		
		RS485_Receive_Data(rs485_rxbuf,len);
		sprintf(humi_data,"%d",rs485_rxbuf[3]<<8|rs485_rxbuf[4]);
		save_data[7] = atoi(humi_data);

		showNumber(40,6,atoi(humi_data)/10,DEC,2,FONT_16_EN);
		
		showNumber(60,6,atoi(humi_data)%10,DEC,2,FONT_16_EN);
		
		save_data[0] = calendar.w_year;
		save_data[1] = calendar.w_month;
		save_data[2] = calendar.w_date;
		save_data[3] = calendar.hour;
		save_data[4] = calendar.min;
		save_data[5] = calendar.sec;

		key_event_hadler();		//按键事件处理
		
		


//		LED_B = !LED_B;
//		LED0 = !LED0;
//		LED1 = !LED1;
//		LED2 = !LED2;
//		LED3 = !LED3;

	} 
}




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
u8 last_mode;

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
 	
	while(1)
	{
		if(sys_mode == USUAL_MODE)
		{
			if(last_mode != USUAL_MODE)
			{
				formatScreen(0X00);
			}
			showCNString(0,3,"温度",FONT_16_CN);
			showString(33,3,":",FONT_16_EN);
			showCNString(0,6,"湿度",FONT_16_EN);
			showString(33,6,":",FONT_16_EN);
			
			RS485_Send_Data(tempbuf,8);// 获取温度
			delay_ms(200);		
			RS485_Receive_Data(rs485_rxbuf,len);
			sprintf(temp_data,"%d",rs485_rxbuf[3]<<8|rs485_rxbuf[4]);
			save_data[6] = atoi(temp_data);

			showNumber(40,3,atoi(temp_data)/10,DEC,2,FONT_16_EN);
			showString(56,3,".",FONT_16_EN);
			showNumber(60,3,atoi(temp_data)%10,DEC,2,FONT_16_EN);
			showCNString(68,3,"镀",FONT_16_EN);		//°



			RS485_Send_Data(humibuf,8);// 获取湿度
			delay_ms(200);		
			RS485_Receive_Data(rs485_rxbuf,len);
			sprintf(humi_data,"%d",rs485_rxbuf[3]<<8|rs485_rxbuf[4]);
			save_data[7] = atoi(humi_data);

			showNumber(40,6,atoi(humi_data)/10,DEC,2,FONT_16_EN);
			showString(56,6,".",FONT_16_EN);
			showNumber(60,6,atoi(humi_data)%10,DEC,2,FONT_16_EN);
			showString(68,6,"%",FONT_16_EN);
			
			save_data[0] = calendar.w_year;
			save_data[1] = calendar.w_month;
			save_data[2] = calendar.w_date;
			save_data[3] = calendar.hour;
			save_data[4] = calendar.min;
			save_data[5] = calendar.sec;
			save_data[6] = atoi(temp_data);
			save_data[7] = atoi(humi_data);
			if(save_data[5] == 0 || save_data[5] == 30)
			{
				
				STMFLASH_Write(FLASH_SAVE_ADDR + SIZE*data_write_count,save_data,SIZE);
				data_write_count++;
				if(data_write_count > 100) //最大100组数据
				{
					data_write_count = 0;
				}
			}		
			last_mode = USUAL_MODE;
	  }
	  
	  else if(sys_mode == ADJUST_TEMP_MODE)
	  {		
		  if(last_mode == USUAL_MODE)
	  	 {			
			formatScreen(0X00);
		 }
			
		  if(sensor.adjust_temp_limit == UP_LIMIT)
		  {
		  	
		  	showCNString(0,3,"温度上限",FONT_16_CN);
		 	showNumber(72,3,sensor.upper_temp_limit,DEC,2,FONT_16_EN);
		  }
		  else if(sensor.adjust_temp_limit == LOW_LIMIT)
		  {
			
		  	showCNString(0,3,"温度下限",FONT_16_CN);
		 	showNumber(72,3,sensor.lower_temp_limit,DEC,2,FONT_16_EN);
		  }
		 last_mode = ADJUST_TEMP_MODE;
	  }

	  else if(sys_mode == ADJUST_HUMI_MODE)
	  {		
		  if(last_mode == USUAL_MODE)
	  	 {			
			formatScreen(0X00);
		 }
			
		  if(sensor.adjust_humi_limit == UP_LIMIT)
		  {
		  	
		  	showCNString(0,3,"湿度上限",FONT_16_CN);
		 	showNumber(72,3,sensor.upper_humi_limit,DEC,2,FONT_16_EN);
		  }
		  else if(sensor.adjust_humi_limit == LOW_LIMIT)
		  {
			
		  	showCNString(0,3,"湿度下限",FONT_16_CN);
		 	showNumber(72,3,sensor.lower_humi_limit,DEC,2,FONT_16_EN);
		  }
		 last_mode = ADJUST_HUMI_MODE;
	  }

	} 
}




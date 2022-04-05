#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"	 								  



extern u8 RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
extern u8 RS485_RX_CNT;   			//接收到的数据长度

//模式控制
#define RS485_TX_EN		PDout(7)	//485模式控制.0,接收;1,发送.
//如果想串口中断接收，请不要注释以下宏定义
#define EN_USART2_RX 	1			//0,不接收;1,接收.

struct sensor_t
{
	u8 upper_temp_limit;	//温度上限
	u8 lower_temp_limit;	//温度下限
	u8 upper_humi_limit;	//湿度上限
	u8 lower_humi_limit;	//湿度下限
	u8 adjust_temp_limit;	//调整温度上下限
	u8 adjust_humi_limit;	//调整湿度上下限
};
extern struct sensor_t sensor;

void RS485_Init(u32 bound);
void RS485_Send_Data(u8 *buf,u8 len);
void RS485_Receive_Data(u8 *buf,u8 *len);


#endif	   

















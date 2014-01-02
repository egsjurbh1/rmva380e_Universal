/**
 * \file      	EE3_uart_process.c
 * \author    	Leo Bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	UART数据接收模块。
**/

#include "EE3_common.h"

Uint8 uint8485RcvBuf[512];
Uint8 uint8232_1_RcvBuf[512];
Uint8 uint8232_2_RcvBuf[512];

/**
 * \function 	hwiUartRerdyFunc
 * \brief    	串口中断响应函数(硬件中断)
 * \			每当串口数据传输给DSP后，该中断会被系统自动调用		
 * \note		1	用户需在使用该中断之前初始化串口
 * \			2	建议用户将串口数据解析处理部分放在相应的线程中执行
**/	
void hwiUartRerdyFunc()
{
	Uint32 cnt;
	//触发读取UART接受缓存的信号灯
	if( Roseek_IsRS485RcvReady() ){
		//读取UART接受缓存
		cnt = Roseek_GetRS485_Recvcnt();
		Roseek_RS485_ReadBuf( (char*)uint8485RcvBuf, cnt );
		Roseek_RS485_WriteData( (char*)uint8485RcvBuf, cnt );
	}
	if( Roseek_IsRS232_1_RcvReady() ){
		//读取UART接受缓存
		cnt = Roseek_GetRS232_1_Recvcnt();
		Roseek_RS232_ReadBuf( Roseek_RS232_PORT1, (char*)uint8232_1_RcvBuf, cnt );
		Roseek_RS232_WriteData( Roseek_RS232_PORT1, (char*)uint8232_1_RcvBuf, cnt );
	}
	if( Roseek_IsRS232_2_RcvReady() ){
		//读取UART接受缓存
		cnt = Roseek_GetRS232_2_Recvcnt();
		Roseek_RS232_ReadBuf( Roseek_RS232_PORT2, (char*)uint8232_2_RcvBuf, cnt );
		Roseek_RS232_WriteData( Roseek_RS232_PORT2, (char*)uint8232_2_RcvBuf, cnt );
	}
}


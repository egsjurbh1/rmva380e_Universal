/**
 * \file      	EE3_pio.c
 * \author    	Leo Bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	可编程IO的处理模块
**/

#include "EE3_common.h"

/***********************外部变量*************************/
extern Uint8 ui8SPIO1Config[];
extern Uint8 ui8SPIO2Config[];
extern RoseekImgInfStruct g_ImgInfStruct;
extern RoseekTrgCtrlInfStruct g_TrgCtrlInfStruct;
/***********************全局变量*************************/
//PIO中断状态信息结构体
typedef struct SPIOIntInf{
	Uint32	ui32IntOccur;			//是否发生中断，1->发生中断，0->没有发生中断
	Uint32	ui32IntEdgePolarity;	//若发生中断，1->上升沿中断，0->下降沿中断
	Uint32	ui32IntTime;			//若发生中断，中断时间，单位为us
}EagleEye2SPIOIntInf;
//PIO中断信息结构体
EagleEye2SPIOIntInf IntePIOIntInf[12] = {0};
//IO中断计时器,缓存线圈触发时的时间，一般用来计算车速
Uint32	ui32IO1IntTime = 0;
Uint32	ui32IO2IntTime = 0;
//触发模式下临时触发方式缓存(软触发，单线圈触发，双线圈触发)，在祯中断中填充祯信息
volatile Uint8	g_ui8TmpTrgMode = 0;//0->软触发，1->单线圈触发，2->双线圈触发
//触发模式下临时车速缓存，在祯采集完毕中断中填充祯信息
volatile float	g_fTmpSpeed = 0.0;
//触发模式下临时抓拍时间缓存，在祯采集完毕中断中填充祯信息
volatile Uint8	g_ui8TmpTrgTime[8];
//自测光触发周期计数
volatile Uint32 g_ui32ADLITrgCnt = 0;
/********************************************************/


/** 	
 * \function 	hwiExpSynIntFunc
 * \brief    	同步曝光中断响应函数(硬件中断)
 * \			每当传感器开始曝光采集一帧图像数据信息时，该中断响应函数会被系统同时调用		
 * \note		1	该中断响应函数被调用的前提是用户在系统初始化模块中调用	Roseek_Enable_ExpSynInt();
 * \			2	建议用户在该中断响应函数中调用系统时间RTC读取
**/
void hwiExpSynIntFunc()
{
	int iTrgSur;
	//如果是软触发，提取抓拍时间
	iTrgSur = Roseek_Get_TrgSurFlag();
	if( iTrgSur>=0 ){//确认为触发
		//读取抓拍时间
		Roseek_RTC_ReadTime( (Uint8*)g_ui8TmpTrgTime );
		//自测光触发周期计数清零
		g_ui32ADLITrgCnt = 0;
		//若为软触发，标示必要标示
		if( iTrgSur==TRGSUR_SOFT ){
			//软触发下车速标示为0
			g_fTmpSpeed = 0.0;
			//表示本祯图像为软触发
			g_ui8TmpTrgMode = 0;
		}
	}
}

/** 	
 * \function 	hwiReadPIOIntFunc
 * \brief    	I/O中断响应函数(硬件中断)
 * \			I/O被配置成输入并使能输入中断，每当外部接口的电路发生0-5V的电平变化时，该中断响应函数会被系统同时调用		
 * \note		1	12个可编程IO的响应事件共用一个中断，用户需在该函数内进行判断
 * \			2	该中断被调用的前提是相应IO被配置成输入，并使能输入中断 或者用户使能了该中断的触发模式抓拍允许事件通知功能
**/
void hwiReadPIOIntFunc()
{
	//process interrupt there
	//1. 确认是否为切换到触发模式后的触发允许

	//2. 处理通用I/O中断
	Roseek_Get_PIOIntInf( (Uint32*)IntePIOIntInf );
	//处理IO1中断
	if( !ui8SPIO1Config[3] ){
		//下降沿中断
		if( IntePIOIntInf[0].ui32IntOccur && !IntePIOIntInf[0].ui32IntEdgePolarity ){
			ui32IO1IntTime = IntePIOIntInf[0].ui32IntTime;
		}
	}
	else{
		//上升沿中断
		if( IntePIOIntInf[0].ui32IntOccur && IntePIOIntInf[0].ui32IntEdgePolarity ){
			ui32IO1IntTime = IntePIOIntInf[0].ui32IntTime;
		}
	}
	//处理IO2中断
	if( !ui8SPIO2Config[3] ){
		//下降沿中断
		if( IntePIOIntInf[1].ui32IntOccur && !IntePIOIntInf[1].ui32IntEdgePolarity ){
			ui32IO2IntTime = IntePIOIntInf[1].ui32IntTime;
			Roseek_RTC_ReadTime( g_ImgInfStruct.ui8TimeInf );
			SEM_post( &sem_CalSpeed );
		}
	}
	else{
		//上升沿中断
		if( IntePIOIntInf[1].ui32IntOccur && IntePIOIntInf[1].ui32IntEdgePolarity ){
			ui32IO2IntTime = IntePIOIntInf[1].ui32IntTime;
			Roseek_RTC_ReadTime( g_ImgInfStruct.ui8TimeInf );
			SEM_post( &sem_CalSpeed );
		}
	}
}

/** 	
 * \function 	tskCalSpeedFunc
 * \brief    	线圈测速
 * \
**/
void tskCalSpeedFunc()
{
	Uint32 ui32TimeDef;

	while(1){
		SEM_pend( &sem_CalSpeed, SYS_FOREVER );
		//若外触发前自测光抓拍已经发生，进行标示视本祯数据有效
		if( g_TrgCtrlInfStruct.bADLIOccur ){
			g_TrgCtrlInfStruct.bExtTrgOccurWhenAutoDetcLig = TRUE;
		}
		else{
			g_TrgCtrlInfStruct.bExtTrgOccurWhenAutoDetcLig = FALSE;
		}
		//计算车速
		//表示本祯图像为硬触发
		if( g_TrgCtrlInfStruct.ui8IsSingalCoin ){
			//如果为单线圈，则不计算车速
			g_ui8TmpTrgMode = 1;
			g_fTmpSpeed = 0.0;
			continue;
		}
		//双线圈触发,计算车速
		g_ui8TmpTrgMode = 2;
		if( ui32IO2IntTime < ui32IO1IntTime ){
			ui32TimeDef = ( 0xffffffff - ui32IO1IntTime ) + ui32IO2IntTime;
		}
		else{
			ui32TimeDef = ui32IO2IntTime - ui32IO1IntTime;
		}
		if( ui32TimeDef < 1 ){
			g_fTmpSpeed = 0.0;
		}
		else{
			g_fTmpSpeed = (g_TrgCtrlInfStruct.fInteval * 3600000) / ui32TimeDef;
		}
	}
}



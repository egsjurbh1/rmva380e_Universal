/**
 * \file      	EE3_ImgAcq.c 
 * \author    	LQ
 * \version   	0.1.3
 * \date      	2013.12.31
 * \brief     	图像采集模块(各图像处理线程触发)
 * \update      修正触发              
**/

#include "EE3_common.h"

/**************************************************************************/
//外部变量
extern volatile Bool            g_bIsDpDataClientConnect;
extern volatile	Bool			g_bIsTrgClientConnect;
extern volatile	Bool			g_bIsFconClientConnect;
extern volatile	Uint8			g_ui8TmpTrgMode;
extern volatile float			g_fTmpSpeed;
extern volatile Uint8			g_ui8TmpTrgTime[8];
extern volatile Bool            BIsTrgFrameofBack;
extern RoseekImgBufStruct		g_ImgBufStruct[]; //图像原始数据缓存结构体
extern RoseekImgInfStruct		g_ImgInfStruct;  //抓拍信息结构体
extern RoseekTrgCtrlInfStruct 	g_TrgCtrlInfStruct; //抓拍控制信息结构体
/**************************************************************************/
//函数声明
int hwiImgAcqOverProcessFunc();
/**************************************************************************/
volatile enum Roseek_SensorRunMode enumRunMode;
Uint8 g_ui8BufIndex = 0;
Uint32 B1YRGB[4],B2YRGB[4],B3YRGB[4],B4YRGB[4],B5YRGB[4];
float ftmp;
/**************************************************************************/
/**
 * \function 	hwiFrameValidFunc
 * \brief    	帧有效中断响应函数(硬件中断)
 * \			每当一帧图像数据数据从FPGA开始传输给DSP的开始，该中断会被系统自动调用		
 * \note		1	用户需在中断响应函数内将用来存放图像原始数据的buffer指针配置到EDMA的配置函数中
 * \			2	不允许用户更改本函数名
**/					
int hwiFrameValidFunc()
{
	Uint32 i,j;
	int iTrgSur;
	enum Roseek_ImgAcquisition_Format enumImgFormat;
	static Bool bFreeBufferHasBeenFound = FALSE;
	Uint8 ui8ScaleLel;

	//判断图像采集处于何种模式
	enumRunMode = Roseek_Get_CurRunMode();
	enumImgFormat = Roseek_Get_CurImgFormat();
	ui8ScaleLel = Roseek_Get_TrgFconMode_ScaleLevel();	

	//读取触发源标示，仅在触发模式下有效
	iTrgSur = Roseek_Get_TrgSurFlag();
	//寻找空缓存
	j = 0;
	bFreeBufferHasBeenFound = FALSE;
	for( i=g_ui8BufIndex; i<g_ui8BufIndex+6; i++ )
	{
		j++;
		if( i>5 ){
			if( g_ImgBufStruct[i-6].bProcessLocked ){   //缓存数据加锁标志，TRUE表示数据正在被处理，FALSE表示数据空闲
			}
			else{
				bFreeBufferHasBeenFound = TRUE;
				g_ui8BufIndex = i-6;
				break;
			}
		}
		else{
			if( g_ImgBufStruct[i].bProcessLocked ){
			}
			else{
				bFreeBufferHasBeenFound = TRUE;
				g_ui8BufIndex = i;
				break;
			}
		}
	}
	if( ( j==6 )&&(!bFreeBufferHasBeenFound) )  //遍历后没有找到空缓存
	{
		return 0;
	}

	g_ImgBufStruct[g_ui8BufIndex].ui8RotateFlag = Roseek_Get_ImgRotateMode();//查询当前图像采集旋转角度

	if( enumRunMode==TrgMode ){
		if( g_TrgBrightnessAdj_Params.bIsAllowBrightnessAdj ){
			if( ((g_TrgBrightnessAdj_Params.ui32PIShutterWidthHigh_us - g_TrgBrightnessAdj_Params.ui32CurrentShutterWidth_us ) <= 200) ){//进入夜间模式
				Roseek_Enable_TrgFlashlamp_Syn_Out();
				Roseek_Set_ImgEnhancement_Level( IMG_ENHANCEMENT_LEVEL1 );
				g_TrgCtrlInfStruct.bEnableADLI = FALSE;
			}
			else{
				Roseek_Set_ImgEnhancement_Level( IMG_ENHANCEMENT_LEVEL3 );
				Roseek_Disable_TrgFlashlamp_Syn_Out();
				g_TrgCtrlInfStruct.bEnableADLI = TRUE;
			}
		}
	}
	g_ImgBufStruct[g_ui8BufIndex].CurImg_CorrespondingRunMode = enumRunMode;
	g_ImgBufStruct[g_ui8BufIndex].CurImg_Format = enumImgFormat;
	g_ImgBufStruct[g_ui8BufIndex].iTrgSurFlag = iTrgSur;
	g_ImgBufStruct[g_ui8BufIndex].ui8TrgFconScaleLel = ui8ScaleLel;
	//配置EDMA进行原始数据的搬移P16函数34）
	Roseek_ImgAcqChannel_Config( enumRunMode, enumImgFormat, g_ImgBufStruct[g_ui8BufIndex].pui8FrameBuf, EDMA_OPT_PRI_LOW );
	return 0;
}


/**
 * \function 	hwiImgAcqOverProcessFunc
 * \brief    	数据采集完毕中断(硬件中断)
 * \			每当一帧图像数据数据被EDMA模块搬移到用户指定的buffer中后，该中断会被系统自动调用		
 * \			通知用户图像数据已经被存放在指定的buffer中，用户可在该函数中将已经采集的图像的相关描述信息填写到图像数据的结构体中
 * \note		1	该中断函数被有效调用的前提是真有效中断中的Roseek_ImgAcqChannel_Config被有效配置
 * \			2   不允许用户更改本函数名
 * \            3   定时检测网络状态
**/	
int hwiImgAcqOverProcessFunc()
{
	Bool	bClientConnect = FALSE;
	//进入采集完毕中断首先备份当前运行模式和缓存索引
	enum Roseek_SensorRunMode enumCurRunMode;
	Uint8 g_ui8CurBufIndex;
	enumCurRunMode = enumRunMode;
	g_ui8CurBufIndex = g_ui8BufIndex;
	//清理EDMA采集通道
	Roseek_Clear_ImgAcqChannel();
	//判断是否进行亮度自适应调节
	Roseek_Get_MIBlockYRGBMean( 1, B1YRGB ); //P15函数28 读取测光区域的平均亮度值
	switch( enumCurRunMode ){
		case TrgMode:
			//若为触发模式，请在此将AFE的增益重新设置一遍
			if( g_TrgBrightnessAdj_Params.bIsAllowBrightnessAdj ){//是否允许电子快门自动调节，TRUE:允许; FALSE:不允许
				Roseek_DetectIllum_Adj( enumCurRunMode, *B1YRGB );
			}
			break;
		case FconMode:
			if( g_FconBrightnessAdj_Params.bIsAllowBrightnessAdj ){
				Roseek_DetectIllum_Adj( enumCurRunMode, *B1YRGB );
			}
			break;
		default:
			break;
	}
	//是否自动白平衡
	if( g_CameraGeneralCtrlParams.bRunWhitebalance ){
		Roseek_WhiteBalance();//执行白平衡操作
	}
	//对触发模式进行判断
	if( enumCurRunMode==TrgMode )
	{
		//为有效抓拍数据填充必要的信息
		g_ImgBufStruct[g_ui8CurBufIndex].ui8TrgMode = g_ui8TmpTrgMode;//触发方式
		memcpy( g_ImgBufStruct[g_ui8CurBufIndex].ui8TrgShotTime, (Uint8*)g_ui8TmpTrgTime, 8 );//抓拍时间
		switch( g_ui8TmpTrgMode )
		{
			case 0://软触发
			case 1://单线圈触发
				g_ImgBufStruct[g_ui8CurBufIndex].fSpeed = 0.0;
				break;
			case 2://双线圈触发
				g_ImgBufStruct[g_ui8CurBufIndex].fSpeed = g_fTmpSpeed;
				break;
			default:
				g_ImgBufStruct[g_ui8CurBufIndex].fSpeed = 0.0;
		}
		
	}
	
	//确定当前采集的图像的运行模式
	switch( g_ImgBufStruct[g_ui8CurBufIndex].CurImg_CorrespondingRunMode ){//当前图像在何种模式下采集(TrgMode, FconMode, HfrMode)
		case TrgMode:
			bClientConnect = g_bIsTrgClientConnect;
			break;
		case FconMode:
			bClientConnect = g_bIsFconClientConnect;
			break;
		default:
			break;
	}
	//1、激活图像处理模块	
    SEM_post(&sem_AcmdBDReady);   	
    //当前帧缓存加锁
	g_ImgBufStruct[g_ui8CurBufIndex].bProcessLocked = TRUE;	 
     
	//2、激活JPEG模块                                       
 	if( bClientConnect)
 	{                                        
 		SEM_post(&sem_JpegReady); //JPEG开始信号量            
 	} 
 	                                                         	                                                         	

    return 0;
}





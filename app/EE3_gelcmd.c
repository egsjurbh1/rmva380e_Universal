/**
 * \file      	EE3_gelcmd.c
 * \author    	Leo bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	网络通用命令传输模块
**/

#include "EE3_common.h"

/**************************************************************************/
#define	CMDHEADLEN	6
/**************************************************************************/
//外部变量
extern volatile	Bool		g_bIsStopTrgSvr;
extern volatile	Bool		g_bTrgSvrIsLink;
extern RoseekImgInfStruct	 	g_ImgInfStruct;
extern RoseekTrgCtrlInfStruct 	g_TrgCtrlInfStruct;
extern AT88SC_Config AT88SC25616C_ConfigZone;
extern volatile Uint8 g_ui8SDFSCMD;
extern char g_szParam1[];
extern char g_szParam2[];
extern volatile Uint8 g_ui8SDFSCMD;
extern char g_szParam1[];
extern char g_szParam2[];

/**************************************************************************/

/**************************************************************************/
//文件局部变量
Uint8 cmdhead[6];
Uint8 bufSnd[256];
Uint8 bufRcv[5000];
Uint32 g_ui32FlashPulsWidth = 0;
volatile    Bool     g_bPolarizer = FALSE;
/**************************************************************************/

/**************************************************************************/
//函数声明
//用来设置触发模式下的控制参数,用户可进行修改
void Roseek_Set_TrgCtrlParam( Uint8* );
/**************************************************************************/

/**************************************************************************/
//外部函数声明
extern Bool Roseek_Encryption_Read_Configuration( Uint8, Uint8, Uint8 );
extern Bool Roseek_Encryption_Initialization( Uint8, Uint8, Uint8 );
/**************************************************************************/
/**
 * \function 	GeneralCmdSrv
 * \brief    	使用TCP/IP协议实现网络命令的接收、解析与参数回读发送
 * \			默认作为服务器端，Port = 35000		
 * \
**/	
void GeneralCmdSrv()
{
	SOCKET	sockCmdListen, sockCmdSvr;
	struct	sockaddr_in addr;
	int		size;
	int		i,cnt,paramlen;
	Uint32	temp = 0;
	Bool	bIsError = FALSE;
	static  Bool bThereIsNoCMD = FALSE;
	static  Uint8 ui8Flag = 0;
	//为当前任务配置运行环境
	fdOpenSession( TaskSelf() );

	//创建命令交互的socket对象
	sockCmdListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( sockCmdListen == INVALID_SOCKET ){
		Roseek_Reset();//如果创建侦听对象失败，重启相机
	}
	bzero( &addr, sizeof(struct sockaddr_in) );
	addr.sin_family	= AF_INET;
	addr.sin_len	= sizeof( addr );
	addr.sin_port	= htons( 35000 );
	//sockCmdListen绑定
	if ( bind( sockCmdListen, (PSA) &addr, sizeof(addr) ) < 0 ){
		Roseek_Reset();//如果绑定失败，重启相机
	}
	//sockCmdListen开始监听，同一时刻仅支持一个连接
	if ( listen( sockCmdListen, 1) < 0 ){
		Roseek_Reset();//如果侦听失败，重启相机
	}
	//迭代接受上位机命令
	for(;;)
	{
		size = sizeof( addr );
		sockCmdSvr = accept( sockCmdListen, (PSA)&addr, &size );
		if( sockCmdSvr == INVALID_SOCKET ){
			//如果接受连接出错则关闭接受到的连接对象，则等待新的连接连接
			fdClose(sockCmdSvr);
			continue;
		}
		//接受命令头，获得命令ID，是否回应标志和参数长度
		i = 0;
		while( i < CMDHEADLEN ){
			cnt = recv( sockCmdSvr, (char*)cmdhead+i, CMDHEADLEN-i, 0 );
			if( cnt <= 0 ){
				fdClose(sockCmdSvr);
				bIsError = TRUE;
				break;
			}
			i += cnt;
		}
		if( bIsError ){
			//如果接受数据出错，则等待新的连接连接
			bIsError = FALSE;
			continue;
		}
		paramlen = (cmdhead[5]<<24) + (cmdhead[4]<<16) + (cmdhead[3]<<8) + cmdhead[2];
		//开始接受命令参数
		i = 0;
		if( paramlen > 0 ){
			while( i < paramlen ){
				cnt = recv( sockCmdSvr, (char*)bufRcv+i, paramlen-i, 0 );
				if( cnt <= 0 ){
					fdClose(sockCmdSvr);
					bIsError = TRUE;
					break;
				}
				i += cnt;
			}
			if( bIsError ){
				//如果接受数据出错，则等待新的连接连接
				bIsError = FALSE;
				continue;
			}
		}
		bThereIsNoCMD = FALSE;
		//解析命令
		switch( cmdhead[0] )
		{
			case 0x00 :
			{
				//系统复位
				Roseek_Reset();
				break;
			}
			case 0x01 :
			{
				//切换运行模式
				temp = bufRcv[0];
				if(ui8Flag == 0)
				{
					ui8Flag = 1;
					Roseek_Set_TrgFconMode_ScaleLevel(FconMode,TRGFCON_SCALE_LEVEL_1);
				}
				else
				{
					ui8Flag = 0;
					Roseek_Set_TrgFconMode_ScaleLevel(FconMode,TRGFCON_SCALE_LEVEL_0);
				}
				switch(temp)
				{
					case 0:
						Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, 1 );
						break;
					case 1:
						Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, 1 );
						break;
					case 2:
						Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, 0 );
						break;
					case 3:
						Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, 0 );
						break;
					default:
						Roseek_Mode_Switch_Ctrl( FconMode, EE3_380_DRCLK_HF );

				
				}
				//Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, EE3_310_DRCLK_LF );
				break;
			}
			case 0x02 :
			{
				//上位机软触发抓取一祯图像
				Roseek_Grab_Single_Frame();
				break;
			}
			case 0x03 :
			{
				/////////////////////读取相机的控制状态\\\\\\\\\\\\\\\\\\\\\\\
				//1.读取当前传感器信号增益值
				*(float*)(bufSnd+4) = g_CameraGeneralCtrlParams.fCurSignalGain;
				//2.读取当前看门狗定时器装载值
				*(Uint32*)(bufSnd+8) = g_CameraGeneralCtrlParams.ui32WDTValue;
				//3.读取闪光灯同步输出使能标志
				*(bufSnd+12) = 1;
				//4.读取切换到触发模式后是否允许相机自动触发抓取一祯图像控制标志
				if( g_CameraGeneralCtrlParams.bAllowAutoTrgAfterModeSwitch ){
					*(bufSnd+13) = 1;
				}
				else{
					*(bufSnd+13) = 0;
				}
				//5.读取是否启用图像LUT映射变换标志
				if( g_CameraGeneralCtrlParams.bIsEnableImgLUT ){
					*(bufSnd+14) = 1;
				}
				else{
					*(bufSnd+14) = 0;
				}
				//6.读取继电器状态
				*(bufSnd+15) = 0;
				//7.读取看门狗使能标志
				if( g_CameraGeneralCtrlParams.bIsEnableWatchDog ){
					*(bufSnd+16) = 1;
				}
				else{
					*(bufSnd+16) = 0;
				}
				//8.读取当前白平衡光照环境
				*(bufSnd+17) = 0;
				//9.读取当前运行模式
				switch(g_CameraGeneralCtrlParams.CurRunModeFlag ){
					case TrgMode:
						*(bufSnd+18) = 0;
						break;
					case FconMode:
						*(bufSnd+18) = 1;
						break;
					default:
						break;
				}
				//10.读取当前图像采集格式
				switch(g_CameraGeneralCtrlParams.CurImgFormat ){
					case RGB_ARRAY:
						*(bufSnd+19) = 0;
						break;
					case YUV_ARRAY:
						*(bufSnd+19) = 1;
						break;
					case BAYER_ARRAY:
						*(bufSnd+19) = 2;
						break;
					default:
						break;
				}
				//11.读取可编程I/O的方向
				*(Uint32*)(bufSnd+20) = 0x00;
				//12.读取可编程I/O的输出值
				*(Uint32*)(bufSnd+24) =0x00;
				//配置回传参数长度
				*(Uint32*)bufSnd = 24;
				break;
			}
			case 0x04 :
			{
				//设置触发模式下的曝光时间
				Roseek_Set_TrgCtrlParam( bufRcv );
				break;
			}
			case 0x05 :
			{
				//设置全分辨率连续模式下的曝光时间
				temp = bufRcv[0] + (bufRcv[1]<<8) + (bufRcv[2]<<16);
				Roseek_Set_FConShutter_Time( temp );
				break;
			}
			case 0x06 :
			{
							break;
			}
			case 0x07 :
			{
				//设置传感器信号增益
				if( *(float*)bufRcv >= PGA_LOWLIMIT && *(float*)bufRcv <= PGA_HIGHLIMIT ){
					Roseek_Set_Afe_Pga( g_CameraGeneralCtrlParams.CurRunModeFlag,*(float*)bufRcv );
					break;
				}
				break;
			}
			case 0x08 :
			{
				//设置白平衡光照条件
				switch( bufRcv[0]){
					case 0:	//Daylight
						g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
						Roseek_Set_WBEvr( DAYLIGHT );
						break;
					case 1: //Fluorescence
						g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
						Roseek_Set_WBEvr( FLUORESCENCE );
						break;
					case 2: //without WhiteBalance
						g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
						Roseek_Set_WBEvr( WITHOUTWB );
						break;
					default:
						break;
				}
				break;
			}
			case 0x09 :
			{
				//是否自动白平衡
				if( bufRcv[0] ){
					g_CameraGeneralCtrlParams.bRunWhitebalance = TRUE;
				}
				else{
					g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
				}
				break;
			}
			case 0x0a :
			{
				//设置是否使能图像LUT映射变换
				if( bufRcv[0] ){
					Roseek_Enable_Image_LUT( );
				}
				else{
					if( bufRcv[1] ){
						Roseek_Disable_Image_LUT( );
					}
					else{
						Roseek_Disable_Image_LUT( );
					}
				}
				break;
			}
			case 0x0b :
			{
				//重置图像LUT映射表
				Roseek_Set_Image_LUT( bufRcv, 0 );
				break;
			}
			case 0x0c :
			{
				//设置闪光灯同步输出使能
				temp = bufRcv[0];
				if( temp==1 ){
					Roseek_Enable_TrgFlashlamp_Syn_Out();
				}
				else{
					Roseek_Disable_TrgFlashlamp_Syn_Out();
				}
				break;
			}
			case 0x0d :
			{
				//设置可编程I/O口的方向
				break;
			}
			case 0x0e :
			{
				//设置可编程I/O的输出值
				//temp的D0~D9 -> Port1~Port10，1为高电平，0为低电平，仅对输出口有效
				temp = (bufRcv[1]<<8) + bufRcv[0];
//				Roseek_Set_Pio_Outvalue( temp );
				break;
			}
			case 0x0f :
			{
				//设置全分辨率模式快门值和增益值
				if( bufRcv[0]==1){
					g_FconBrightnessAdj_Params.bIsAllowBrightnessAdj = TRUE;
					Roseek_Set_Afe_Pga( FconMode,g_FconBrightnessAdj_Params.fCurrentAfePga_db );
					Roseek_Set_FConShutter_Time( g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us );
				}
				else{
					g_FconBrightnessAdj_Params.bIsAllowBrightnessAdj = FALSE;
				}
				break;
			}
			case 0x10 :
			{
				//设置全分辨率连续模式下的相机进行亮度自动调节的曝光时间上限
				g_FconBrightnessAdj_Params.ui32PIShutterWidthHigh_us = (bufRcv[2]<<16) + (bufRcv[1]<<8) + (bufRcv[0]);
				g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us = g_FconBrightnessAdj_Params.ui32PIShutterWidthHigh_us;
				g_FconBrightnessAdj_Params.fCurrentAfePga_db = PGA_LOWLIMIT;
				Roseek_Set_Afe_Pga( FconMode,g_FconBrightnessAdj_Params.fCurrentAfePga_db );
				Roseek_Set_FConShutter_Time( g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us );
				break;
			}
			case 0x11 :
			{
							break;
			}
			case 0x12 :
			{
								break;
			}
			case 0x13 :
			{
				//设置相机进行亮度自动调节的信号增益上限
				g_TrgBrightnessAdj_Params.fPIPgaHigh_db = *(float*)bufRcv;
				g_FconBrightnessAdj_Params.fPIPgaHigh_db = *(float*)bufRcv;
				break;
			}
			case 0x14 :
			{
				//设置全分辨率连续模式下相机进行亮度自动调节的图像亮度阈值范围
				g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold = (Uint8)bufRcv[0];
				if( g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold > 235){
					g_FconBrightnessAdj_Params.ui32PIYmeanHighThreshold = 255;
				}
				else{
					g_FconBrightnessAdj_Params.ui32PIYmeanHighThreshold = g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold + 15;
				}
				if( g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold < 30){
					g_FconBrightnessAdj_Params.ui32PIYmeanLowThreshold = 10;
				}
				else{
					g_FconBrightnessAdj_Params.ui32PIYmeanLowThreshold = g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold - 15;
				}
				break;
			}
			case 0x15 :
			{
							break;
			}
			case 0x16 :
			{
				//设置是否使能看门狗
				if( bufRcv[0] ){
					Roseek_Watchdog_Open();
					Roseek_KickWatchdog();
				}
				else{
					Roseek_Watchdog_Close();
				}
				break;
			}
			case 0x17 :
			{
				//设置看门狗定时器
				Roseek_Set_WatchdogTimer( bufRcv[0] );
				break;
			}
			case 0x18 :
			{
				//设置相机的实时时钟时间
				bufRcv[7] = 0;
				Roseek_RTC_SetTime( bufRcv );
				break;
			}
			case 0x19 :
			{
				//读取相机的实时时钟时间
				Roseek_RTC_ReadTime( bufSnd+4 );
				*(Uint32*)bufSnd = 7;
				Roseek_Waitusec( 10 );
				break;
			}
			case 0x1a :
			{
				//修改相机的IP地址
				temp = (bufRcv[3]<<24) + (bufRcv[2]<<16) + (bufRcv[1]<<8) + (bufRcv[0]);
				Roseek_Save_IP_Address( temp );
				break;
			}
			case 0x1b :
			{
				//修改相机的MAC地址
				bufRcv[6] = 0x00;
				bufRcv[7] = 0x00;
				Roseek_Save_MAC_Address( bufRcv );
				Roseek_Waitusec( 15000);
				break;
			}
			case 0x1c :
			{
				//读取相机的MAC地址
				Roseek_Read_MAC_Address( bufSnd+4 );
				*(Uint32*)bufSnd = 6;
				Roseek_Waitusec( 10 );
				break;
			}
			case 0x1d :
			{
				//设置继电器闭合或断开，1为闭合，0为断开
				break;
			}
			case 0x1e :
			{
				//读取相机温度
				*(float*)( bufSnd+4 ) = Roseek_Read_CurTemperature();
				*(Uint32*)bufSnd = 4;
				Roseek_Waitusec( 10 );
				break;
			}
			case 0x1f :
			{
				//设置切换到触发模式后是否允许相机自动触发抓取一祯图像
				if( bufRcv[0] ){
					Roseek_Enable_RunModeSwitch_AutoTrg( TRUE );
				}
				else{
					Roseek_Enable_RunModeSwitch_AutoTrg( FALSE );
				}
				break;
			}
			case 0x20 :
			{
				//切换图像的采集格式
				switch( bufRcv[0] ){
					case 0:
						Roseek_Set_ImgAcquisition_Format( RGB_ARRAY );
						break;
					case 1:
						Roseek_Set_ImgAcquisition_Format( YUV_ARRAY );
						break;
					case 2:
						Roseek_Set_ImgAcquisition_Format( BAYER_ARRAY );
						break;
					default:
						break;
				}
				break;
			}
			case 0x21 :
			{
				//停止触发模式下的上传服务
				g_bIsStopTrgSvr = TRUE;
				SEM_post( &sem_TrgDataReady );
				break;
			}
			case 0x22 :
			{
				//设置触发模式下的快门速度是否与其他两种模式保持一致
				if( bufRcv[0] ){
					Roseek_SetTrgShutterTimeCtrlMode( TRUE );
				}
				else{
					Roseek_SetTrgShutterTimeCtrlMode( FALSE );
				}
				break;
			}
			case 0x23 :
			{
				//检测触发模式上传服务器是否正常
				if( g_bTrgSvrIsLink ){
					g_bIsStopTrgSvr = TRUE;
					SEM_post( &sem_TrgDataReady );
				}
				break;
			}
			case 0x24 :
			{
								break;
			}
			case 0x25 :
			{
								break;
			}
			case 0x26 :
			{
						break;
			}
			case 0x27 :
			{
								break;
			}
			case 0x2E :
			{
				//读取用户加密芯片配置区内容
				if( Roseek_Encryption_Read_Configuration( bufRcv[0], bufRcv[1], bufRcv[2] )){
					memcpy(bufSnd+4,&AT88SC25616C_ConfigZone,sizeof(AT88SC_Config));
					*(Uint32*)bufSnd = 256;
				}
				else{
					*(Uint32*)bufSnd = 4;
				}
				break;
			}
			case 0x2F :
			{
				//配置用户加密芯片配置区内容
				memcpy( &AT88SC25616C_ConfigZone, bufRcv+4, sizeof(AT88SC_Config));
				if( Roseek_Encryption_Initialization( bufRcv[0], bufRcv[1], bufRcv[2] )){
					*(Uint32*)bufSnd = 8;
				}
				else{
					*(Uint32*)bufSnd = 4;
				}
				break;
			}
			case 0x30:  //光学机构切换控制
			{       
				temp = bufRcv[0];       
				if(temp==0)       
				{            
					g_bPolarizer = FALSE;            
					Roseek_Drive_LF_Switch(SWITCH_TO_GENERAL_FILTER);       
				}       
				else       
				{            
					g_bPolarizer = TRUE;            
					Roseek_Drive_LF_Switch(SWITCH_TO_POLARIZER_FILTER);
				}       
				break; 
			} 
			case 0x31:  //闪光灯输出控制 
			{       
				temp = bufRcv[0];       
				if(temp==0)       
				{             
					Roseek_Disable_TrgFlashlamp_Syn_Out();             
					Roseek_Disable_FconFlashlamp_Syn_Out();       
				}       
				else       
				{             
					Roseek_Enable_TrgFlashlamp_Syn_Out();             
					Roseek_Enable_FconFlashlamp_Syn_Out();       
				}       
				break; 
			}
			case 0x32:   //设置补光灯输出的有效脉宽 
			{       
				g_ui32FlashPulsWidth = bufRcv[0]*255+bufRcv[1];       
				if(g_ui32FlashPulsWidth>=799)       
				{          
					g_ui32FlashPulsWidth =799;       
				}       
				Roseek_Set_FlashSynWidth(g_ui32FlashPulsWidth);       
				break; 
			}
			case 0x33:  //可视化的LUT更改 
			{       
				if(bufRcv[0] ==1)       
				{            
					Roseek_Set_Image_LUT( bufRcv+1, 1);//设置LUT 并将其存储到EEPROM当中       
				}       
				else       
				{            
					Roseek_Set_Image_LUT( bufRcv+1, 0);//设置LUT 不存储       
				}        
				break; 
			} 
			case 0x34: 
			{       
				Roseek_Resume_Image_LUT();//恢复LUT为出厂默认值       
				break; 
			}

			case 0x35:
			{
				Roseek_Get_CPU_ID(bufSnd+4);
				*(Uint32*)bufSnd = 6;
				break; 
			}

			case 0xff :
			{
				g_ui8SDFSCMD = (Uint8)bufRcv[0];
				memcpy(g_szParam1, bufRcv + 1, 17);
				memcpy(g_szParam2, bufRcv + 18, 17);
				g_szParam1[16] = '\0';
				g_szParam2[16] = '\0';
				
				break;
			}
			default :
			{
				bThereIsNoCMD = TRUE;
				break;
			}
		}
		//向上位机回传数据
		if( cmdhead[1] ){
			if(bThereIsNoCMD)
			{
				memset(bufSnd,0,256);			
			}
			if( send( sockCmdSvr, (char*)bufSnd, *(Uint32*)bufSnd + 4, 0 ) < 0 ){
				//如果发送出错，则等待新的连接连接
				fdClose( sockCmdSvr );
				continue;
			}
		}
		//关闭连接，回迭代循环
		fdClose( sockCmdSvr );
	}//迭代循环
}

//设置触发模式下的控制参数
void Roseek_Set_TrgCtrlParam( Uint8* pui8ParamSur )
{
	Uint32	i,tmp1,tmp2,tmp3,tmp4;
	float	ftmp;
	Uint8	ui8Tmp;
	Bool	bSaveParam = FALSE;

	//抓拍地点
	i = 0;
	memcpy( g_ImgInfStruct.ui8LocusInf, pui8ParamSur, 100 );
	g_ImgInfStruct.ui8LocusInf[100] = '\0';
	i += 101;
	//车道号
	memcpy( &g_ImgInfStruct.ui8WayNum, pui8ParamSur+i, 1 );
	i += 1;
	//触发方式
	if( pui8ParamSur[i]==1 ){
		//单线圈触发
		g_TrgCtrlInfStruct.ui8IsSingalCoin = 1;
		i += 1;
	}
	else{
		//双线圈触发
		g_TrgCtrlInfStruct.ui8IsSingalCoin = 0;
		i += 1;
		memcpy( &g_TrgCtrlInfStruct.fInteval, pui8ParamSur+i, 4 );
	}
	i += 4;
	//快门增益控制
	if( !pui8ParamSur[i] ){
		//手动控制
		i += 1;
		memcpy( &tmp1, pui8ParamSur+i, 4 );//快门值
		g_TrgBrightnessAdj_Params.ui32CurrentShutterWidth_us = tmp1;
		Roseek_Set_TrgShutter_Time( tmp1 );
		i += 4;
		memcpy( &ftmp, pui8ParamSur+i, 4 );//增益值
		g_TrgBrightnessAdj_Params.fCurrentAfePga_db = ftmp;
		Roseek_Set_Afe_Pga( TrgMode,ftmp );
		i += 4;
		//禁止自测光和快门增益自动控制
		g_TrgCtrlInfStruct.bEnableADLI = FALSE;
		g_TrgBrightnessAdj_Params.bIsAllowBrightnessAdj = FALSE;
		//是否保存参数
		i += 35;
		if( (Uint8)pui8ParamSur[i] ){
			bSaveParam = TRUE;
		}
		else{
			bSaveParam = FALSE;
		}
	}
	else{
		//自动控制
		i += 9;
		memcpy( &tmp1, pui8ParamSur+i, 4 );//快门上限
		i += 4;
		g_TrgBrightnessAdj_Params.ui32PIShutterWidthHigh_us = tmp1;
		memcpy( &ftmp, pui8ParamSur+i, 4 );//增益上限
		g_TrgBrightnessAdj_Params.fPIPgaHigh_db = ftmp;
		i += 4;
		memcpy( &tmp1, pui8ParamSur+i, 4 );//当前快门
		g_TrgBrightnessAdj_Params.ui32CurrentShutterWidth_us = tmp1;
		Roseek_Set_TrgShutter_Time( tmp1 );
		i += 4;
		memcpy( &ftmp, pui8ParamSur+i, 4 );//当前增益
		g_TrgBrightnessAdj_Params.fCurrentAfePga_db = ftmp;
		Roseek_Set_Afe_Pga( TrgMode,ftmp );
		i += 4;
		memcpy( &tmp1, pui8ParamSur+i, 4 );//测光区域起始行坐标
		i += 4;
		memcpy( &tmp2, pui8ParamSur+i, 4 );//测光区域起始列坐标
		i += 4;
		memcpy( &tmp3, pui8ParamSur+i, 4 );//测光区域终止行坐标
		i += 4;
		memcpy( &tmp4, pui8ParamSur+i, 4 );//测光区域终止列坐标
		i += 4;
		Roseek_Set_MIBlockParam( 1, tmp1, tmp2, tmp3, tmp4 );
		g_TrgBrightnessAdj_Params.ui32PICurYmeanThreshold = (Uint8)pui8ParamSur[i];
		if( g_TrgBrightnessAdj_Params.ui32PICurYmeanThreshold > 235){
			g_TrgBrightnessAdj_Params.ui32PIYmeanHighThreshold = 255;
		}
		else{
			g_TrgBrightnessAdj_Params.ui32PIYmeanHighThreshold = g_TrgBrightnessAdj_Params.ui32PICurYmeanThreshold + 15;
		}
		if( g_TrgBrightnessAdj_Params.ui32PICurYmeanThreshold < 30){
			g_TrgBrightnessAdj_Params.ui32PIYmeanLowThreshold = 10;
		}
		else{
			g_TrgBrightnessAdj_Params.ui32PIYmeanLowThreshold = g_TrgBrightnessAdj_Params.ui32PICurYmeanThreshold - 15;
		}
		i += 1;
		//是否使能自测光
		if( (Uint8)pui8ParamSur[i] ){
			g_TrgCtrlInfStruct.bEnableADLI = TRUE;
		}
		else{
			g_TrgCtrlInfStruct.bEnableADLI = FALSE;
		}
		i += 1;
		//自测光周期
		g_TrgCtrlInfStruct.ui8AutoDetcLigIllumPeriod = (Uint8)pui8ParamSur[i];
		g_TrgCtrlInfStruct.ui32AutoDetcLigIllumCNT = g_TrgCtrlInfStruct.ui8AutoDetcLigIllumPeriod;
		i += 1;
		//使能快门增益自动控制
		g_TrgBrightnessAdj_Params.bIsAllowBrightnessAdj = TRUE;
		//是否保存参数
		if( (Uint8)pui8ParamSur[i] ){
			bSaveParam = TRUE;
		}
		else{
			bSaveParam = FALSE;
		}
	}
	//是否保存参数
	if( bSaveParam ){
		//保存抓拍信息
		Roseek_Flash_Read( &ui8Tmp, TRGINFFLAGADD, 1 );
		if( ui8Tmp == 0xff ){
			ui8Tmp = 0x55;
			Roseek_Flash_Burn( &ui8Tmp, TRGINFFLAGADD, 1 );
		}
		Roseek_Flash_Burn( (Uint8*)&g_ImgInfStruct, TRGINFADD, sizeof(RoseekImgInfStruct) );
		Roseek_Flash_Burn( (Uint8*)&g_TrgCtrlInfStruct, TRGCTRLINFADD, sizeof(RoseekTrgCtrlInfStruct) );
		Roseek_Flash_Burn( (Uint8*)&g_TrgBrightnessAdj_Params, TRGCTRLPARAMADD, sizeof(RoseekImgYMeanAutoAdjParams) );
	}
}


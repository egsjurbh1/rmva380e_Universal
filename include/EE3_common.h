//////////////////////////////////////////////////////////////////////
// File name: EE3_common.h      		                            //    
//////////////////////////////////////////////////////////////////////

/**
 * \file      	EE3_common.h
 * \author    	LQ
 * \version   	0.05
 * \date      	2012.12.1
 * \brief     	DSP Demo程序中的用到的宏定义，和相关结构体。
 *				该文件中的内容允许用户根据需要自定义更改。
 *				用户创建的源文件中需要包含本头文件
 *              ！更新：加入结果结构体
**/

#ifndef	_EE3_COMMON_H
#define	_EE3_COMMON_H

#include <std.h>
#include <tsk.h>
#include <c64.h>
#include <stdlib.h>
#include <csl_types.h>
#include <soc.h>
#include <bcache.h>
#include <cslr.h>
#include <stdio.h>
#include <log.h>
#include <tistdtypes.h>
#include <cslr_cache.h>
#include <c6x.h>
#include <cslr_i2c.h>
#include <cslr_pllc1.h>
#include <cslr_tmr.h>
#include <cslr_psc.h>
#include <cslr_sys.h>
#include <string.h>
#include <math.h>
#include <netmain.h>
#include "EE3E_380_lib.h"
#include "EE3cfg.h"
#include "roseek_ee3_rtsp.h"


//图像处理算法的行列分辨率	
#define ALG_ROW                         544u   
#define ALG_COL                         960u 
//功能Mode定义
#define TEST_MODE                       0       //无图像处理功能模式（测试用）
#define QUEUE_DETECT					1      	//功能一：队长检测
#define VEHICLE_DETECT					2		//功能二：车辆检测
#define WORD_NUM 	7

/**********车辆检测有关***********/
#define NUM_RUMTIME 	100     	//统计运行次数
#define QUEUE_LENGTH 	11		//高斯滤波长度
#define COIL_NUM     	2  		//线圈数
#define FRAMEOFBACK_NUM	5		//每5帧一次背景建模	

/***与后台控制系统的通信协议***/
#define ACMDPORT 	6366		//端口
#define HEADCODE 	0xFFFF		//协议头
#define NETREQUEST 	100			//向摄像头请求队长
#define NETRETURN  	101			//摄像头返回队长
#define NETINFORTG 	103			//通知红转绿
#define NETINFOGTR 	104	    	//通知绿转红

/***与远程配置工具的通信协议***/
//通信端口
#define CONTROLPORT 4500
#define VIDEOPORT 	4501
#define UDPPORT   	4502 
//配置命令 
#define BROADCAST1 	   	0
#define GETBACKGROUND  	1  
#define SETPOINT    	3   //虚拟线圈配置命令
#define REBOOT      	4   //重启相机
#define STARTVIDEO  	7   //开始发送Video
#define ENDVIDEO    	8   //停止发送Video
#define CHOOSECAM   	10  //选择相机
#define EE3_SENDCONFIG	16  //相机参数配置发送
#define EE3_SETCONFIG	17	//相机参数配置接受
#define CANCELCMD    	20  //取消当前操作
#define UDPCON     		99  //UDP请求
#define UDPSETTIME 		100 //UDP广播系统时间
/***相机配置信息保存地址，自定义***/
#define	CFGCOMMONADD					0x102000	//length = 80 Byte，存储通用配置信息
#define CFGVDADD						0x102100	//length = 30 Byte，存储车辆检测基本配置
#define CFGQDADD						0x102200	//length = 30 Byte，存储队长检测基本配置
#define CFGVDPOINTADD					0x102300	//length = 80 Byte，存储车辆检测虚拟线框配置
#define CFGQDPOINTADD					0x102400	//length = 80 Byte，存储队长检测虚拟线框配置

/***抓拍信息保存地址,DEMO使用，用户可修改***/
#define	TRGINFFLAGADD					0x000000	//length = 1 Byte
#define	TRGINFADD						0x000001	//length = 256 Byte
#define	TRGCTRLINFADD					0x000101	//length = 256 Byte
#define	TRGCTRLPARAMADD					0x000201	//length = 256 Byte
#define	NETPORTINITFLAGADD				0x000301	//length = 1 Byte, 0x55用来表示已存信息,系统上电将读出已存信息来进行初始化,否则将使用默认信息初始化
#define	NETPORTINFADD					0x000302	//length = 12 Byte, 保存顺序依次为Trg,Fcon,Hfr;各占用4个字节，为无符号整型数

/**************************************************************************/
//图像原始数据缓存结构体，其实例化对象用来存储采集模块接收到的图像原始数据及相关信息
typedef struct ImgBufStruct{
	Uint8	*pui8FrameBuf;				//图像缓存
	Uint8	ui8RotateFlag;				//图像旋转标志
	Uint8	ui8TrgFconScaleLel;			//触发和全分辨率连续模式下的缩放级别
	Bool	bProcessLocked;				//缓存数据加锁标志，TRUE表示数据正在被处理，FALSE表示数据空闲
	int		iTrgSurFlag;				//若图像采集于触发模式下，表示本祯图像来自于哪个触发源
	Uint8	ui8TrgShotTime[8];			//触发模式下的抓拍时间，年-月-日-星期-时-分-秒-十倍毫秒，连续模式下不使用，无效
	Uint8	ui8TrgMode;					//触发方式，0->软触发;1->单线圈;2->双线圈(测速)，连续模式无效
	float	fSpeed;						//触发模式下车辆速度，仅双先圈触发有效，连续模式无效
	Bool    iTrgFrameofBack;             //是否作为背景图像
	enum Roseek_SensorRunMode CurImg_CorrespondingRunMode;	//当前图像在何种模式下采集(TrgMode, FconMode, HfrMode)
	enum Roseek_ImgAcquisition_Format CurImg_Format;			//当前图像为何种模式(RGB_ARRAY, YUV_ARRAY, BAYER_ARRAY)
}RoseekImgBufStruct;
/**************************************************************************/

/**************************************************************************/
//JPEG图像缓存结构体，其实例化对象用来存储JPEG编码模块用到的中间数据，以及输出的JPEG图像数据
typedef struct JPGEncBufStruct{
	Uint8	*pui8YCrCb_YBuf;			//编码器输入图像的Y分量缓存指针
	Uint8	*pui8YCrCb_CrBuf;			//编码器输入图像的Cr分量缓存指针
	Uint8	*pui8YCrCb_CbBuf;			//编码器输入图像的Cb分量缓存指针
	Uint8	*pui8JpgImgBuf[2];			//编码器输出的非触发方式JPEG图像的缓存序列
	Uint8	*pui8TrgJpgImgBuf[6];		//编码器输出的触发方式JPEG图像的缓存序列
	Bool	bJpgImgBuf_Lock[2];			//编码器输出的非触发方式JPEG图像的缓存序列的数据有效性标志位
	Bool	bTrgJpgImgBuf_Lock[6];		//编码器输出的触发方式JPEG图像的缓存序列数据有效性标志位
}RoseekJPGEncBufStruct;
/**************************************************************************/

/**************************************************************************/
//DEMO中给出的定义，共114个字节，用户可根据实际情况修改
//抓拍信息结构体，请实例化对象用于存储上传的图像描述信息
typedef struct ImgInfStruct{
	char	ui8LocusInf[101];			//抓拍地点信息，限50个汉字
	Uint8	ui8TimeInf[8];				//抓拍时间，年-月-日-星期-时-分-秒-10倍毫秒
	Uint8	ui8WayNum;					//车道号
	Uint8	ui8TrgMode;					//触发方式，0->软触发;1->单线圈;2->双线圈(测速)
	float	fSpeed;						//车速(单位:公里/小时)   
	Bool    bIsTrgpic;                  //是否作为背景图像
}RoseekImgInfStruct;
/**************************************************************************/

/**************************************************************************/
//DEMO中给出的定义，用户可根据实际情况修改
//抓拍控制信息结构体
typedef struct TrgCtrlInfStruct{
	Bool	bEnableADLI;				//是否允许自测光
	Uint8	ui8AutoDetcLigIllumPeriod;	//自测光抓拍周期(单位:秒)
	Uint32	ui32AutoDetcLigIllumCNT;	//自测光抓拍计数器触发值(由自测光抓拍周期换算得到，DSP端定义的计数器递增周期为1秒)
	Bool	bADLIOccur;					//自测光标志
	Bool	bExtTrgOccurWhenAutoDetcLig;//自测光期间是否发生外触发
	Uint32	ui32FixedShutterTime;		//不需要自测光时的固定快门时间(单位:微秒)
	float	fFixedPGA;					//不需要自测光时的固定增益(单位:dB)
	Uint8	ui8IsSingalCoin;			//是否单线圈系统
	float	fInteval;					//如果是双线圈系统，线圈间距(单位:米)	          
}RoseekTrgCtrlInfStruct;

/**************************************************************************
********图像处理部分
**************************************************************************/

//图像处理数据缓冲，实例化对象用来存储前景与背景图像的YUV分量
typedef struct ImgBufStructITS{
        Uint8   *pui8FrameBuf;              //原始图像YUV422缓存
        Uint8	*pui8YCrCb_YBuf;			//原始图像转为YUV420后的Y分量缓存指针
	    Uint8	*pui8YCrCb_CrBuf;			//原始图像转为YUV420后的Cr分量缓存指针
	    Uint8	*pui8YCrCb_CbBuf;			//原始图像转为YUV420后的Cb分量缓存指针
}ITS_ImgBufStruct;
//建模四边形结构体,用于算法部分
typedef struct Point{
	int xMax;    
	int xMin;
	int yMax;
	int yMin;    //建立矩形框端点坐标
	int pt5x;
	int pt5y;
	int pt6x;
	int pt6y;    //四边形两对边的中点坐标
	float k1;
	float k2;
	float k3;
	float k4;
	float k5;
	float b1;
	float b2;
	float b3;
	float b4;
	float b5;    //用四条y=kx+b直线在矩形框范围内构建一个四边形，k3为中线
}DpPoint;
//算法部分使用
typedef struct LineInfo{
	int foreground_count;//前景数
	int total_pixel;//像素总值
}Lineinfo;
//队列检测算法部分所需缓存
typedef struct algfuncData{
    Uint8       *ay_data; 
    Lineinfo    *aforeground;
	float       *aflag;
}AlgfuncData;
//车辆检测算法部分所需缓存
typedef struct VDalgfuncData{
	Uint8      *f_data;
	Uint8      *pro_imgback;
	Uint8      *tmpdata1;
	Uint8      *tmpdata2;
}VDalgfuncData;
//背景建模算法部分所需缓存
typedef struct bpalgData{
    float   *ww;
	float   *mean;
	float   *sd;
}BpalgData;
//数车结构体
typedef struct carInfo{
	int num[5];
	float speed[5];
}CarInfo;
/**************************************************************************
******后台控制通?
**************************************************************************/

//与后台控制端通信，控制信息存储结构体
typedef struct CmdInputData{
    Uint8 	head1;   		//协议头
	Uint8 	head2;			//协议头
	Uint8 	length;   		//长度
	Uint8 	label1;       	//标记
	Uint8 	label2;       	//标记
	Uint8  	cameraN;		//车道号
	Uint8  	comm;			//命令	
	Uint8   agree;			//标记队长结果是否正确，正确为1，错误为0
	Uint8 	dataR;			//填充
	Uint8   data;			//队列长度
}PcInData;

//传输信息通用结构体
typedef struct CmdUData{
	float  nSumRT;     //运行时间
	float  nFrame;	   //每秒处理帧数
}CmdUData;

//当前控制信息结构体
typedef struct ACmdSem{
	Uint8  	comm;			//命令
	Bool    bStart;			//启动标记
}AcmdSem;

//发送的车辆信息结构体
typedef struct sendInfo{
	Int label;
	Int num;
	Float speed;
	Char local_ip[20];//new
}SendInfo;

/**************************************************************************
*******远程配置
**************************************************************************/

//与远程配置工具通信，控制命令存储结构体
typedef struct conversationHead{
	int command;         //命令
	int bufsize;         //长度
}ConversationHead;

//相机当前配置信息结构体
typedef struct eE3_CfgStruct{
		int 	RunMode;         		//功能模式（0 无；1 功能一 ；2功能二）
		char	CameraIp[20];       	//相机当前IP地址
		char    ServerIp[20];			//管理平台IP
		int  	ServerPort;				//管理平台端口号
		int  	RoadNum[2];				//监控车道编号（仅功能二）
		int		UploadTime;				//上传周期（仅功能一）
		int  	NightTrafficStream;		//夜晚车流（仅功能一）		
		Uint8 	RealTime[8];			//相机系统时间
		int     Isreadflash;			//是否读取了Flash中的配置		
}EE3_CfgStruct;
//相机当前参数结构体
typedef struct EE3_StateStruct{
		char 	Version[40];			//主程序版本号
		int  	Resolution;				//相机分辨率（1 960*544； 2 1920*1080）
}EE3_StateStruct;

/**************************************************************************/

/**************************************************************************/
//加密芯片AT88SCXXC配置区结构体
typedef struct {
	Uint8 AnswerToReset[8];		//00-07
	Uint8 FabCode[2];			//08-09
	Uint8 MTZ[2];				//0A-0B
	Uint8 CMC[4];				//0C-0F
	Uint8 LostHistoryCode[8];	//10-17
	Uint8 DCR[1];				//18
	Uint8 IdentificationCode[7];//19-1F
	///////////////////////
	Uint8 AR0[1];				//20
	Uint8 PR0[1];				//21
	Uint8 AR1[1];				//22
	Uint8 PR1[1];				//23
	Uint8 AR2[1];				//24
	Uint8 PR2[1];				//25
	Uint8 AR3[1];				//26
	Uint8 PR3[1];				//27
	Uint8 AR4[1];				//28
	Uint8 PR4[1];				//29
	Uint8 AR5[1];				//2A
	Uint8 PR5[1];				//2B
	Uint8 AR6[1];				//2C
	Uint8 PR6[1];				//2D
	Uint8 AR7[1];				//2E
	Uint8 PR7[1];				//2F
	Uint8 AR8[1];				//30
	Uint8 PR8[1];				//31
	Uint8 AR9[1];				//32
	Uint8 PR9[1];				//33
	Uint8 AR10[1];				//34
	Uint8 PR10[1];				//35
	Uint8 AR11[1];				//36
	Uint8 PR11[1];				//37
	Uint8 AR12[1];				//38
	Uint8 PR12[1];				//39
	Uint8 AR13[1];				//3A
	Uint8 PR13[1];				//3B
	Uint8 AR14[1];				//3C
	Uint8 PR14[1];				//3D
	Uint8 AR15[1];				//3E
	Uint8 PR15[1];				//3F
	///////////////////////
	Uint8 IssuerCode[16];		//40-4F
	///////////////////////
	Uint8 ACC0[1];				//50
	Uint8 CryptoCi0[7];			//51-57
	Uint8 SessionKeySk0[8];		//58-5F
	Uint8 ACC1[1];				//60
	Uint8 CryptoCi1[7];			//61-67
	Uint8 SessionKeySk1[8];		//68-6F
	Uint8 ACC2[1];				//70
	Uint8 CryptoCi2[7];			//71-77
	Uint8 SessionKeySk2[8];		//78-7F
	Uint8 ACC3[1];				//80
	Uint8 CryptoCi3[7];			//81-87
	Uint8 SessionKeySk3[8];		//88-8F
	///////////////////////
	Uint8 SecretSeedG0[8];		//90-97
	Uint8 SecretSeedG1[8];		//98-9F
	Uint8 SecretSeedG2[8];		//A0-A7
	Uint8 SecretSeedG3[8];		//A8-AF
	///////////////////////
	Uint8 PAC_Write0[1];		//B0
	Uint8 WritePassword0[3];	//B1-B3
	Uint8 PAC_Read0[1];			//B4
	Uint8 ReadPassword0[3];		//B5-B7
	Uint8 PAC_Write1[1];		//B8
	Uint8 WritePassword1[3];	//B9-BB
	Uint8 PAC_Read1[1];			//BC
	Uint8 ReadPassword1[3];		//BD-BF
	///////////////////////
	Uint8 PAC_Write2[1];		//C0
	Uint8 WritePassword2[3];	//C1-C3
	Uint8 PAC_Read2[1];			//C4
	Uint8 ReadPassword2[3];		//C5-C7
	Uint8 PAC_Write3[1];		//C8
	Uint8 WritePassword3[3];	//C9-CB
	Uint8 PAC_Read3[1];			//CC
	Uint8 ReadPassword3[3];		//CD-CF
	///////////////////////
	Uint8 PAC_Write4[1];		//D0
	Uint8 WritePassword4[3];	//D1-D3
	Uint8 PAC_Read4[1];			//D4
	Uint8 ReadPassword4[3];		//D5-D7
	Uint8 PAC_Write5[1];		//D8
	Uint8 WritePassword5[3];	//D9-DB
	Uint8 PAC_Read5[1];			//DC
	Uint8 ReadPassword5[3];		//DD-DF
	///////////////////////
	Uint8 PAC_Write6[1];		//E0
	Uint8 WritePassword6[3];	//E1-E3
	Uint8 PAC_Read6[1];			//E4
	Uint8 ReadPassword6[3];		//E5-E7
	Uint8 PAC_Write7[1];		//E8
	Uint8 WritePassword7[3];	//E9-EB
	Uint8 PAC_Read7[1];			//EC
	Uint8 ReadPassword7[3];		//ED-EF
	///////////////////////
	Uint8 SystemReserved[16];	//F0-FF
} AT88SC_Config;
/**************************************************************************/

#endif




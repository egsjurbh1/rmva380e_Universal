/**
 * \file      	EE3_GlobalVariables.c
 * \author    	LQ
 * \version   	0.1.0
 * \date      	2013.12.31
 * \brief     	相机运行时使用的全局变量定义及初始化源文件
 * \update      图像buffer初始化
**/

#include "EE3_common.h"

/**************************************************************************/
//图像原始数据缓冲，系统默认设置6个buffer，见mian.c中的内存分配
RoseekImgBufStruct	g_ImgBufStruct[6];
//队列检测算法部分结构体,有内存分配
AlgfuncData g_algfuncData;
//车辆检测算法部分结构体，有内存分配
VDalgfuncData g_VDalgfuncData;
//背景建模算法部分结构体,有内存分配
BpalgData g_bpalgData;  //第一路
BpalgData g_bpalgData2;  //第二路
//图像背景数据缓冲, 存储车辆检测VD图像背景数据，有内存分配
ITS_ImgBufStruct  g_ImgBufofBackVD =
{
	NULL,
	NULL,
	NULL,
	NULL
};
//图像背景数据缓冲，存储队长检测QD第一路图像背景数据供图像处理模块使用,有内存分配
ITS_ImgBufStruct  g_ImgBufofBackS =
{
	NULL,
	NULL,
	NULL,
	NULL
};
//图像背景数据缓冲，存储队长检测QD第二路图像背景数据供图像处理模块使用,有内存分配
ITS_ImgBufStruct  g_ImgBufofBackS2 =
{
	NULL,
	NULL,
	NULL,
	NULL
};
//图像前景数据缓冲，存储图像前景数据供图像处理模块使用，有内存分配
ITS_ImgBufStruct  g_ImgBufofFrontS =
{
	NULL,           //*pui8FrameBuf;
	NULL,           //*pui8YCrCb_YBuf;
	NULL,           //*pui8YCrCb_CrBuf;
	NULL            //*pui8YCrCb_CbBuf;
};
//JPG编码维护变量
RoseekJPGEncBufStruct g_JpgEncBufStruct =
{
	NULL,			//*pui8YCrCb_YBuf;
	NULL,			//*pui8YCrCb_CrBuf;
	NULL,			//*pui8YCrCb_CbBuf;
	{NULL},			//*pui8JpgImgBuf[2];
	{NULL},			//*pui8TrgJpgImgBuf[6];
	{FALSE},		//bJpgImgBuf_Lock[2];
	{FALSE}			//bTrgJpgImgBuf_Lock[6];
};

//JPG编码存入SD卡变量
RoseekJPGEncBufStruct g_JpgEncBuftoSD =
{
	NULL,			//*pui8YCrCb_YBuf;
	NULL,			//*pui8YCrCb_CrBuf;
	NULL,			//*pui8YCrCb_CbBuf;
	{NULL},			//*pui8JpgImgBuf[2];
	{NULL},			//*pui8TrgJpgImgBuf[6];
	{FALSE},		//bJpgImgBuf_Lock[2];
	{FALSE}			//bTrgJpgImgBuf_Lock[6];
};

//抓拍控制信息
RoseekTrgCtrlInfStruct g_TrgCtrlInfStruct =
{
	TRUE,			//bEnableADLI
	5,				//ui8AutoDetcLigIllumPeriod
	5,				//ui32AutoDetcLigIllumCNT
	FALSE,			//bADLIOccur
	FALSE,			//bExtTrgOccurWhenAutoDetcLig
	1500,			//ui32FixedShutterTime;
	30.0,			//fFixedPGA
	1,				//ui8IsSingalCoin
	5.0				//fInteval
};

//抓拍信息
RoseekImgInfStruct g_ImgInfStruct =
{
	{"上海市同济大学嘉定校区"},	//ui8LocusInf[102]
	{0},						//ui8TimeInf[8]
	1,							//ui8WayNum
	1,							//ui8TrgMode
	0.0,							//fSpeed
	FALSE                       //非背景图像
};


//接收控制端信息存储结构体
PcInData g_BpInputData =
{
	0xFF,		//协议头
	0xFF,		//协议头
	0x07,		//长度
	0xCC,		//标记
	0xCC,		//标记
	0,			//车道号
	0,   		//命令
	0,			//结果标记
	0x00,     	//填充
	0			//队长
};
//向控制端发送信息存储结构体
PcInData g_BpOutputData1 =
{
	0xFF,		//协议头
	0xFF,		//协议头
	0x07,		//长度
	0xCC,		//标记
	0xCC,		//标记
	0,			//车道号
	0,   		//命令
	0,			//结果标记
	0x00,     	//填充
	0			//队长

};
PcInData g_BpOutputData2 =
{
	0xFF,		//协议头
	0xFF,		//协议头
	0x07,		//长度
	0xCC,		//标记
	0xCC,		//标记
	0,			//车道号
	0,   		//命令
	0,			//结果标记
	0x00,     	//填充
	0			//队长

};

//CmdUData传输信息通用结构体
CmdUData g_CmdUdata =
{
	0,            //总运行时间
	0
};
//第一路当前控制信息结构体
AcmdSem g_AmcdSem1 = 
{
	0,          	//车道号
	FALSE 			//开启标记
};
//第二路当前控制信息结构体
AcmdSem g_AmcdSem2 = 
{
	0,          	//车道号
	FALSE 			//开启标记
};

//接收远程平台控制命令存储结构体
ConversationHead g_DecCmdsem =
{
  0,
  0
};

//数车结构体（5车道）
CarInfo g_carInfo = 
{				
	{0,22,0,0,0},		//车辆数
	{0,22,0,0,0}			//速度
};
//相机当前运行参数状态结构体
EE3_CfgStruct g_EE3Cur = 
{
	0,					//功能模式（0 无；1 功能一 ；2功能二）
	"192.168.1.218",	//相机当前IP地址
	"10.60.139.195",	//管理平台IP
	4545,				//管理平台端口号
	{6,7},				//监控车道编号（仅功能二）
	30,					//上传周期（仅功能一）
	12,					//夜晚车流（仅功能一）
	{0,0,0,0,0,0,0,0},	//相机系统时间
	0					//是否读取了Flash中的配置		
};

volatile Bool IsReadFlash = TRUE;//是否从Flash中读取相机运行参数（默认开启）

//相机当前状态参数
EE3_StateStruct g_EE3State = 
{
	"rmva380e_Universal_V0.29",		//主程序版本号
	1								//相机分辨率（1 960*544； 2 1920*1080）
};

//光耦合输出结构体
RoseekISOOutAtbStruct g_ISOOutAtb =
{
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

DAT_HANDLE			g_hDAT0;
DAT_HANDLE			g_hDAT1;
DAT_HANDLE			g_hDAT2;
DAT_HANDLE			g_hDAT3;
DAT_HANDLE			g_hDAT4;
DAT_HANDLE			g_hDAT5;
DAT_HANDLE			g_hDAT6;
DAT_HANDLE			g_hDAT7;
volatile	Bool	g_bIsTrgClientConnect = FALSE;
volatile 	Bool	g_bIsFconClientConnect = FALSE;
volatile	Bool	g_bIsFcontoDecClientConnect = FALSE;
volatile	Bool	g_bIsHfrClientConnect = FALSE;
volatile	Bool	g_bIsHfrBinClientConnect = FALSE;
volatile	Bool	g_bIsStopTrgSvr = FALSE;
volatile	Bool	g_bTrgSvrIsLink = FALSE;
volatile    Bool    g_bIsSendRoadInfConnect = FALSE;
volatile	Uint8	ui8SPIO1Config[16] = { 0, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile	Uint8	ui8SPIO2Config[16] = { 0, 2, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
volatile	Uint8	ui8SPIOUDConfig[16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


//L1/2 CAHCE SIZE
volatile	Uint32	g_ui32L1PSize = EE3_L1_CACHE_SIZE_32K;
volatile	Uint32	g_ui32L1DSize = EE3_L1_CACHE_SIZE_32K;
volatile	Uint32	g_ui32L2Size = EE3_L2_CACHE_SIZE_128K; 
//CPU Frequency
Uint32 g_ui32CPUCoreClkFrequency = CPU_CORE_CLK_1125MHZ;
//全局变量
volatile    Uint32  CodeRate[2] = {0};    //码率全局存储
volatile    float   Ftmpnow=0;         //当前温度
volatile    Bool    BIsTrgFrameofBack = FALSE;      //是否作为背景图像
volatile 	Bool    g_bIsSendPictoDec = FALSE;		//是否传输图像      
volatile    struct  sockaddr g_ConfigSetaddr;
volatile    int   	bText = 0;
volatile    int     bTextConnect = 0;
volatile    int     bTextCount = 0;
volatile    int     bTextRecev = 0;
volatile    int     PointDataVD[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile    int     PointDataQD[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile    Uint32  nRunT[2];
volatile    float   RunTimeOnce[NUM_RUMTIME];
volatile    float   runTimeT[60];




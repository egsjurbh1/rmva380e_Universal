/**
 * \file      	EE3_main.c 
 * \author    	LQ
 * \version   	0.15
 * \date      	2013.11.24
 * \brief     	RMVA380SC 的系统初始化模块
 * \update      新的IO口初始化
**/

#include "EE3_common.h"

/**************************************************************************/
//外部变量
extern Uint8 ui8SPIO1Config[16];
extern Uint8 ui8SPIO2Config[16];
extern Uint8 ui8SPIOUDConfig[16];
extern RoseekISOOutAtbStruct g_ISOOutAtb;
extern RoseekImgInfStruct g_ImgInfStruct; //抓拍信息结构体,实例化对象用于存储上传的图像描述信息
extern RoseekTrgCtrlInfStruct g_TrgCtrlInfStruct;//抓拍控制信息结构体
extern RoseekImgBufStruct	g_ImgBufStruct[]; //图像原始数据缓存结构体，其实例化对象用来存储采集模块接收到的图像原始数据及相关信息
extern RoseekJPGEncBufStruct g_JpgEncBufStruct; //JPEG图像缓存结构体，其实例化对象用来存储JPEG编码模块用到的中间数据，以及输出的JPEG图像数据
extern RoseekJPGEncBufStruct g_JpgEncBuftoSD;//JPEG图像缓存，其实例化对象存储JPEG编码临时数据
extern ITS_ImgBufStruct   g_ImgBufofBackS; //图像背景数据缓冲，供图像处理模块使用（自定义）
extern ITS_ImgBufStruct   g_ImgBufofBackS2; //第二路（自定义）
extern ITS_ImgBufStruct   g_ImgBufofFrontS; //图像前景数据缓冲，供图像处理模块使用（自定义）
extern ITS_ImgBufStruct   g_ImgBufofBackVD;//图像背景数据缓冲，车辆检测使用
extern AlgfuncData g_algfuncData; //队列检测算法数据缓存
extern VDalgfuncData g_VDalgfuncData;//车辆检测算法数据缓存
extern BpalgData g_bpalgData; //背景建模算法数据缓存，第一路
extern BpalgData g_bpalgData2;  //背景建模算法数据缓存，第二路
extern int	extHeap;
extern DAT_HANDLE g_hDAT0;
extern DAT_HANDLE g_hDAT1;
extern DAT_HANDLE g_hDAT2;
extern DAT_HANDLE g_hDAT3;
extern DAT_HANDLE g_hDAT4;
extern DAT_HANDLE g_hDAT5;
extern DAT_HANDLE g_hDAT6;
extern DAT_HANDLE g_hDAT7;
extern volatile    Uint32  CodeRate[2];
extern volatile    Bool    BIsTrgFrameofBack;
/**************************************************************************/
void Roseek_SysParamInit();
void Roseek_Allocate_Memory();
/**************************************************************************/


/**
 * \function 	main
 * \brief    	EagleEye系列智能相机系统内使用了TI提供的DSP/BIOS是实时操作系统内核，main函数作为系统的初始化函数存在，在DSP/BIOS内核开始工作之前完成系统状态的初始化
 * \note		用户自定义的初始化内容需要在main函数中的 Roseek_SysParamInit（）函数内的Roseek_Initialize_SysBufs（）之后进行	
 * \			
**/	
void main()
{
	Roseek_SysParamInit();
}

/**
 * \function 	Roseek_SysParamInit
 * \brief    	用户参数初始化子函数
 * \note		用户自定义的初始化内容需在该函数内的Roseek_Initialize_SysBufs（）之后进行	
 * \			
**/	
void Roseek_SysParamInit()
{
  	Uint8 ui8Tmp = 0xff;  //设置缓冲区地址	
  	Uint8 i;
	Uint32 codeRate[2] = {0};
	char    cha[104];

	//申请各缓冲区数据存储空间
	Roseek_Allocate_Memory();

	//申请系统使用的缓冲区数据存储空间
	Roseek_Initialize_SysBufs();

	
	//配置H.264视频流压缩板参数,根据当前的视频模式分辨率大小来配置
	Roseek_Set_EncResolution( ENC_380S_DS_1080P_CIF  ); //初始ENC_380S_DS_1080P_CIF
	Roseek_Set_EncCodeRate(8000);   //设置视频输出时的码流大小，100~8000Kb/s，初设8000
	Roseek_Get_ENCCodeRate(&codeRate[0]);
	Roseek_Set_SubCodeRate(400);    //设置CIF格式子视频流的码流大小
	Roseek_Get_SubCodeRate(&codeRate[1]);
	for(i=0;i<2;i++)
	   CodeRate[i]=codeRate[i];
	//视频流上显示含结果信息
	memset( cha, '\0', 104 );
    sprintf(cha, "分辨率:1920*1088,码率:%dKbps",CodeRate[0]);
    Roseek_Set_EncOSDInf( cha );
	Roseek_Set_EncOSD_ShowOp(ENC_OSD_FONT2X,0,0,0,0,0,0,0,0,0);//设定OSD视频文字信息P41函数12
	//sprintf( cha, "EE3相机H.264子码流,分辨率:CIF,码率:%dKbps", CodeRate[1] );
	//Roseek_Set_EncCIFOSDInf( cha );
	
	Roseek_Open_ENCDataChannel();  //打开编码ASIC数据传输通道

	//设置图像色彩饱和度参数
	Roseek_Set_ColorSaturation_Param( 1.6 );  //1.0为不调整色彩饱和度，0为黑白,初设1.3。
	//初始化SD卡
	Roseek_SD_Initialize();
	
	//初始化缓存信息结构体的对象
	for( i=0; i<6; i++ ){
		g_ImgBufStruct[i].bProcessLocked = FALSE;   //缓存数据加锁标志，空闲
		g_ImgBufStruct[i].ui8RotateFlag = IMGR_DEGREE0;  //图像旋转标志
		g_ImgBufStruct[i].ui8TrgFconScaleLel =1;         //触发和全分辨率连续模式下的缩放级别0为1920*1088，1为960*544
		g_ImgBufStruct[i].iTrgSurFlag = 0;               //若图像采集于触发模式下，表示本祯图像来自于哪个触发源
		memset( g_ImgBufStruct[i].ui8TrgShotTime, 0, 8 ); //触发模式下的抓拍时间，年-月-日-星期-时-分-秒-十倍毫秒，连续模式下不使用，无效
		g_ImgBufStruct[i].ui8TrgMode = 0;                 //触发方式，0->软触发;1->单线圈;2->双线圈(测速)，连续模式无效
		g_ImgBufStruct[i].fSpeed = 0.0;                   //触发模式下车辆速度，仅双先圈触发有效，连续模式无效
		g_ImgBufStruct[i].CurImg_CorrespondingRunMode = FconMode; //当前图像在何种模式下采集(TrgMode, FconMode, HfrMode)
		g_ImgBufStruct[i].CurImg_Format = YUV_ARRAY;       //当前图像为何种模式(RGB_ARRAY, YUV_ARRAY, BAYER_ARRAY)
		g_ImgBufStruct[i].iTrgFrameofBack = FALSE;         //当前图像采集是否作为背景图像（new)
	}
	for( i=0;i<6;i++){
		g_JpgEncBufStruct.bTrgJpgImgBuf_Lock[i]=FALSE; //编码器输出的触发方式JPEG图像的缓存序列数据有效性标志位
	}
	
	for( i=0;i<2;i++){
		g_JpgEncBufStruct.bJpgImgBuf_Lock[i]=FALSE;  //编码器输出的非触发方式JPEG图像的缓存序列的数据有效性标志位
	}

	BIsTrgFrameofBack = TRUE; //初设为TRUE，用于触发收集背景图像存入SD卡。
	
	//默认采集模式为YUV点阵
	Roseek_Set_ImgAcquisition_Format( YUV_ARRAY );//初设YUV_ARRAY

	//设置图像增强等级为3
	Roseek_Set_ImgEnhancement_Level(IMG_ENHANCEMENT_LEVEL3); //初设IMG_ENHANCEMENT_LEVEL3
	
	//配置IO1和IO2为1车道两个线圈输入，两个线圈用来测速，2号线圈触发抓拍
	Roseek_SPIOInfConfig( 0, ui8SPIO1Config );//IO1
	Roseek_SPIOInfConfig( 1, ui8SPIO2Config );//IO2
	
	//配置IO3和IO4为两个线框车辆检测输出
	Roseek_SPIOInfConfig( 2, ui8SPIOUDConfig );//IO3
	Roseek_SPIOInfConfig( 3, ui8SPIOUDConfig );//IO4

	//配置IO5和IO6为两个车道队长检测输出，5号对应RoadNum[0]路，6号对应RoadNum[1]路
	Roseek_SPIOInfConfig( 4, ui8SPIOUDConfig );//IO5
	Roseek_SPIOInfConfig( 5, ui8SPIOUDConfig );//IO6

	//配置光耦合输出端口ISO1、ISO2
	Roseek_ISOOUT_InfConfig( 0, g_ISOOutAtb );   //ISO1
	Roseek_ISOOUT_InfConfig( 1, g_ISOOutAtb );   //ISO1
	
	//使能曝光同步中断
	Roseek_Enable_ExpSynInt();

	//从8MB的Flash中读取存储信息，初始化系统运行参数
	Roseek_Flash_Read( &ui8Tmp, TRGINFFLAGADD, 1 ); //从Flash的TRGINFFLAGADD地址开始读1个字节数据到ui8Tmp=0xff缓冲区
	if( ui8Tmp!=0xff ){//0xff表示未保存过抓拍信息
		Roseek_Flash_Read( (Uint8*)&g_ImgInfStruct, TRGINFADD, sizeof(RoseekImgInfStruct) );
		g_ImgInfStruct.ui8LocusInf[100] = '\0'; //抓拍地点信息，限50个汉字,初始化\0
		Roseek_Flash_Read( (Uint8*)&g_TrgCtrlInfStruct, TRGCTRLINFADD, sizeof(RoseekTrgCtrlInfStruct) );
		Roseek_Flash_Read( (Uint8*)&g_TrgBrightnessAdj_Params, TRGCTRLPARAMADD, sizeof(RoseekImgYMeanAutoAdjParams) );
	}

	//关闭触发模式下的闪光灯同步输出
	Roseek_Disable_TrgFlashlamp_Syn_Out();
	//关闭连续模式下的闪光灯同步输出
	Roseek_Disable_FconFlashlamp_Syn_Out();


	
	//配置测光区域p15函数26）
	Roseek_Set_MIBlockParam( 1, 400, 400, 832, 1216 );//初设1, 400, 400, 832, 1216
	//初始化传感器的曝光时间和信号内部增益
	switch( Roseek_Get_CurRunMode() ){
		case TrgMode:
			Roseek_Set_Afe_Pga( TrgMode,g_TrgBrightnessAdj_Params.fCurrentAfePga_db );//设置TrgMode运行模式下传感器输出信号增益
			Roseek_Set_TrgShutter_Time( g_TrgBrightnessAdj_Params.ui32CurrentShutterWidth_us );
			break;
		case FconMode:
			Roseek_Set_Afe_Pga( FconMode,g_FconBrightnessAdj_Params.fCurrentAfePga_db );
			Roseek_Set_FConShutter_Time( g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us );
			break;
		default:
			break;
	}

	//初始化JPG编码器的参数
	Roseek_JPGENC_Init( 2048, 2048 );//待编码图像的最大分辨率为2048*2048

	//初始化串口工作模式
	Roseek_SetRS232RcvBufDepth( Roseek_RS232_PORT1, 256 );
	Roseek_RS232_SetBaudrate( Roseek_RS232_PORT1, Roseek_RS232_BAUDRATE_38400 );
	Roseek_RS232_Open( Roseek_RS232_PORT1 );
	Roseek_SetRS232RcvBufDepth( Roseek_RS232_PORT2, 256 );
	Roseek_RS232_SetBaudrate( Roseek_RS232_PORT2, Roseek_RS232_BAUDRATE_38400 );
	Roseek_RS232_Open( Roseek_RS232_PORT2 );
	Roseek_SetRS485RcvBufDepth( 256 );
	Roseek_RS485_SetBaudrate(Roseek_RS485_BAUDRATE_38400);
	Roseek_RS485_Open();

	//设置DMA模块系统总线优先级，不建议用户修改该API的参数
	Roseek_Set_DMASysPriority(3);

	//申请DAT资源

	g_hDAT0 = Roseek_DAT_Open(QDMA_OPT_PRI_URGENT);
	g_hDAT1 = Roseek_DAT_Open(QDMA_OPT_PRI_URGENT);
	g_hDAT2 = Roseek_DAT_Open(QDMA_OPT_PRI_HIGH);
	g_hDAT3 = Roseek_DAT_Open(QDMA_OPT_PRI_MEDIUM);
	g_hDAT4 = Roseek_DAT_Open(QDMA_OPT_PRI_URGENT);
	g_hDAT5 = Roseek_DAT_Open(QDMA_OPT_PRI_URGENT);
	g_hDAT6 = Roseek_DAT_Open(QDMA_OPT_PRI_HIGH);
	g_hDAT7 = Roseek_DAT_Open(QDMA_OPT_PRI_MEDIUM);
	
	//设置相机初始模式为连续模式,25fps
	Roseek_Mode_Switch_Ctrl( FconMode, EE3_380_DRCLK_HF);//初设FconMode, EE3_380_DRCLK_HF
	//初设置触发模式为全分辨率图像，不经过缩放
	Roseek_Set_TrgFconMode_ScaleLevel(TrgMode,TRGFCON_SCALE_LEVEL_1);//初设TrgMode,TRGFCON_SCALE_LEVEL_0
	//初设置连续模式为全分辨率图像，不经过缩放
	Roseek_Set_TrgFconMode_ScaleLevel(FconMode,TRGFCON_SCALE_LEVEL_1);//初设FconMode,TRGFCON_SCALE_LEVEL_0
	
	//打开图像采集通道
	Roseek_Open_Imagechannel();
}

/**
 * \function 	Roseek_Allocate_Memory
 * \brief    	申请数据缓存
 * \note		用户必须将系统运行时使用的缓存在DSP/BIOS系统运行敖猩昵耄×勘苊庠谠诵兄卸昵搿⑹头糯娴牟僮?
 * \			
**/	
void Roseek_Allocate_Memory()
{
	// 申请图像采集原始数据缓存
	g_ImgBufStruct[0].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[0].pui8FrameBuf==NULL);

	g_ImgBufStruct[1].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[1].pui8FrameBuf==NULL);

	g_ImgBufStruct[2].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[2].pui8FrameBuf==NULL);

	g_ImgBufStruct[3].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[3].pui8FrameBuf==NULL);

	g_ImgBufStruct[4].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[4].pui8FrameBuf==NULL);

	g_ImgBufStruct[5].pui8FrameBuf = MEM_alloc( extHeap,  FINE_ROW*FINE_COL*3, 256);
	while( g_ImgBufStruct[5].pui8FrameBuf==NULL);
	
	// 申请ITS应用图像处理所需的图像数据缓存
	//车辆检测背景
	g_ImgBufofBackVD.pui8YCrCb_YBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL*2, 256);
	while( g_ImgBufofBackVD.pui8YCrCb_YBuf==NULL);
	//队长检测背景
	g_ImgBufofBackS.pui8YCrCb_YBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL, 256);
	while( g_ImgBufofBackS.pui8YCrCb_YBuf==NULL);

	g_ImgBufofBackS2.pui8YCrCb_YBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL, 256);
	while( g_ImgBufofBackS2.pui8YCrCb_YBuf==NULL);
	
    // 前景
	g_ImgBufofFrontS.pui8FrameBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL*3, 256);
	while( g_ImgBufofFrontS.pui8FrameBuf==NULL);

	g_ImgBufofFrontS.pui8YCrCb_YBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL, 256);
	while( g_ImgBufofFrontS.pui8YCrCb_YBuf==NULL);

	g_ImgBufofFrontS.pui8YCrCb_CbBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL, 256);
	while( g_ImgBufofFrontS.pui8YCrCb_CbBuf==NULL);

	g_ImgBufofFrontS.pui8YCrCb_CrBuf = MEM_alloc( extHeap,  ALG_ROW*ALG_COL, 256);
	while( g_ImgBufofFrontS.pui8YCrCb_CrBuf==NULL);

	// 申请"队列检测"算法过程所需缓存
    g_algfuncData.ay_data = MEM_alloc( extHeap,  ALG_COL*ALG_ROW, 256);
	while( g_algfuncData.ay_data==NULL);

	g_algfuncData.aflag = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW, 256);
	while( g_algfuncData.aflag==NULL);

	g_algfuncData.aforeground = MEM_alloc( extHeap,  sizeof(Lineinfo)*ALG_ROW, 256);
	while( g_algfuncData.aforeground==NULL);

	// 申请"车辆检测"算法过程所需缓存
    g_VDalgfuncData.f_data = MEM_alloc( extHeap,  ALG_COL*ALG_ROW, 256);
	while( g_VDalgfuncData.f_data==NULL);

	g_VDalgfuncData.pro_imgback = MEM_alloc( extHeap,  ALG_COL*ALG_ROW, 256);
	while( g_VDalgfuncData.pro_imgback==NULL);

	g_VDalgfuncData.tmpdata1 = MEM_alloc( extHeap,  ALG_COL*ALG_ROW, 256);
	while( g_VDalgfuncData.tmpdata1==NULL);

	g_VDalgfuncData.tmpdata2 = MEM_alloc( extHeap,  ALG_COL*ALG_ROW, 256);
	while( g_VDalgfuncData.tmpdata2==NULL);
	
	// 申请背景建模算法过程所需缓存
	g_bpalgData.mean = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData.mean==NULL);

	g_bpalgData.sd = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData.sd==NULL);

	g_bpalgData.ww = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData.ww==NULL);

	g_bpalgData2.mean = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData2.mean==NULL);

	g_bpalgData2.sd = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData2.sd==NULL);

	g_bpalgData2.ww = MEM_alloc( extHeap,  sizeof(float)*ALG_ROW*ALG_COL*3, 256);
	while( g_bpalgData2.ww==NULL);

	// 申请非TrgMode的JPEG数据所需的缓存
	g_JpgEncBufStruct.pui8JpgImgBuf[0] = MEM_calloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8JpgImgBuf[0]==NULL);

	g_JpgEncBufStruct.pui8JpgImgBuf[1] = MEM_calloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8JpgImgBuf[1]==NULL);

    // 申请TrgMode的JPEG数据所需的缓存
	g_JpgEncBufStruct.pui8TrgJpgImgBuf[0] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[0]==NULL);

	g_JpgEncBufStruct.pui8TrgJpgImgBuf[1] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[1]==NULL);
	
	g_JpgEncBufStruct.pui8TrgJpgImgBuf[2] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[2]==NULL);

	g_JpgEncBufStruct.pui8TrgJpgImgBuf[3] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[3]==NULL);
	
	g_JpgEncBufStruct.pui8TrgJpgImgBuf[4] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[4]==NULL);

	g_JpgEncBufStruct.pui8TrgJpgImgBuf[5] = MEM_alloc( extHeap, FINE_ROW*FINE_COL, 256);
	while( g_JpgEncBufStruct.pui8TrgJpgImgBuf[5]==NULL);

}

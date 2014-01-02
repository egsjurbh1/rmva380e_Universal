/**
 * \file      	ITS_ImgMain.c 
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.3.4
 * \date      	2013.12.31
 * \brief     	智能交通应用----图像处理主线程(车辆检测功能、队列长度检测功能)
 * \update      修正读取配置信息BUG
**/

#include "EE3_common.h"
#include "SDFS_filesys.h"
/**************************************************************************/
extern ITS_ImgBufStruct      g_ImgBufofBackS;
extern ITS_ImgBufStruct      g_ImgBufofBackS2;
extern ITS_ImgBufStruct      g_ImgBufofFrontS;
extern ITS_ImgBufStruct  	 g_ImgBufofBackVD;
extern RoseekImgBufStruct		g_ImgBufStruct[]; //图像原始数据缓存结构体
extern int     PointDataVD[16];
extern int     PointDataQD[16];
extern BpalgData g_bpalgData;
extern BpalgData g_bpalgData2;
extern AcmdSem 		g_AmcdSem1;
extern AcmdSem 		g_AmcdSem2;
extern PcInData     g_BpInputData;
extern PcInData 	g_BpOutputData1;
extern PcInData 	g_BpOutputData2;
extern Uint32  nRunT[2];
extern volatile     float  RunTimeOnce[NUM_RUMTIME];
extern volatile    float   runTimeT[60];
extern CmdUData g_CmdUdata;
extern volatile	Bool	        g_bIsFcontoDecClientConnect;
extern volatile    Bool    g_bIsSendRoadInfConnect;
extern EE3_CfgStruct g_EE3Cur;//相机运行状态
extern volatile Bool IsReadFlash;
extern CarInfo g_carInfo;
/**************************************************************************/
extern int GrayscaleQD(Uint8 *back_data,Uint8 *in_data,DpPoint *pt,Uint32 *firstip,Uint32 *lastip);
extern int GrayscaleVD(Uint8 *in_data, float *ww, float *mean, float *sd, DpPoint *ptP,
		 Uint8 *ptn, Uint8 *carNum,	float *nMat, Uint8 *back_data,	Uint8 numofframe);
extern void gaussian(Uint8 *b_data, Uint8 *i_data, float *ww, float *mean, float *sd,int yMin, int yMax, int xMin, int xMax);
extern void setwwmeansd(Float *ww, Float *mean, Float *sd);
extern void EE3_YUV2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
extern void GaussianPram(Float *nMat, int k);
void DrawPoint(Uint8 *in_data, int *PointData);
void ReadPointDataFromFlash();
void ReadSetInfFormFlash(Bool isReadflash);
void PointDataConfig(DpPoint *pt, int *pointdata);
void VD_SemOut_IO(Uint8 *isC);
void ResetPointData();
void Count_RunTime(Uint32 *nRT);
void ITS_VehicleDetect(DpPoint *dpPVD);
void ITS_QueueDetect(DpPoint  *dpPQD);
int StringConvertInt(char *p);
void Test_PointDPlus();
Bool DecodingCfgData(char *bufstream, int n, char word[][30]);
/**************************************************************************/
/**
 * \function 	tskAcmdBDmainFunc
 * \brief    	图像处理系统线程
 * \			该模块接受ImgAcq中信号触发，每帧图像触发		
 * \            1、		
 * \            2、
**/	
void tskImgmainFunc()
{
	 Uint8   *pui8IMGbuffer;//当前采集帧图像缓存
	 Uint8	 i;
	 DpPoint dpPointVD[2];
	 DpPoint dpPointQD[2];	 
	 Uint8	 ui8tmp = 0;
	 Bool	 bBreak = FALSE;			//切出标志
	 Bool    bReadOnce = TRUE;
	 Bool    bsemSendtoDecready = FALSE;        
	 static	 Uint8	ui8FrameIndex = 0; //帧缓存序号

	 //准备算法所需数据
	 setwwmeansd( g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd);
	 setwwmeansd( g_bpalgData2.ww, g_bpalgData2.mean, g_bpalgData2.sd);	
	 ResetPointData();//线圈清零
	     
     while( 1 )
	 {
	    SEM_pend(&sem_AcmdBDReady,SYS_FOREVER);
		TSK_sleep( 2 );
				
		nRunT[0] = Roseek_GetTickCount();//开始计数
		
		//******读取FLASH中的配置参数，启动后读取一次。
		if(bReadOnce)
		{
			ReadPointDataFromFlash();//读线框配置数据
			PointDataConfig(dpPointVD, PointDataVD);//车辆检测的线框配置
			PointDataConfig(dpPointQD, PointDataQD);//队列检测的线框配置
			ReadSetInfFormFlash(IsReadFlash);//读相机运行参数数据
			//Test_PointDPlus();//测试用，手动设定线框大小
			bReadOnce=FALSE;
		}
			
		//判断当前处理哪个原始图像缓存
		bBreak = FALSE;
		if(ui8FrameIndex==5){
			ui8tmp=0;
		}
		else{
			ui8tmp=ui8FrameIndex+1;
		}
		for( i=ui8tmp; i<ui8tmp+6; i++ ){
			if( i>5 ){
				if( g_ImgBufStruct[i-6].bProcessLocked ){
					ui8FrameIndex = i-6;
					bBreak = TRUE;
					break;
				}
			}
			else{//i<=5
				if( g_ImgBufStruct[i].bProcessLocked ){
					ui8FrameIndex = i;
					bBreak = TRUE;
					break;
				}
			}
		}
		if( !bBreak )
		{
			g_ImgBufStruct[ui8FrameIndex].bProcessLocked = FALSE;
			continue;
		}
		g_ImgBufofFrontS.pui8FrameBuf = g_ImgBufStruct[ui8FrameIndex].pui8FrameBuf;//复制前景图像
		pui8IMGbuffer = g_ImgBufStruct[ui8FrameIndex].pui8FrameBuf;
		bsemSendtoDecready = g_bIsFcontoDecClientConnect; //发送图像线程准备完毕
		//激活向配置工具传输图像线程信号量
		if(bsemSendtoDecready)
		{
	    	SEM_post(&sem_FcontoDecReady);
		}
 		//此处调用画虚拟线框函数		
		switch(g_EE3Cur.RunMode)
		{
			//车辆检测
			case VEHICLE_DETECT:
				DrawPoint(pui8IMGbuffer, PointDataVD);
				break;
			//队长检测
			case QUEUE_DETECT:
				DrawPoint(pui8IMGbuffer, PointDataQD);
				break;
			default:
				break;
		}
		//清除 L2 Cache
		BCACHE_wbAll();

		//******转换原始图像YUV422至YUV420格式，并提取Y分量
		EE3_YUV2YCrCb420(	pui8IMGbuffer,
							g_ImgBufofFrontS.pui8YCrCb_YBuf, g_ImgBufofFrontS.pui8YCrCb_CrBuf, g_ImgBufofFrontS.pui8YCrCb_CbBuf,
							ALG_COL, ALG_ROW );
		
		g_ImgBufStruct[ui8FrameIndex].bProcessLocked = FALSE;//原始图像解锁	

		//功能判断
		switch(g_EE3Cur.RunMode)
		{
			//车辆检测
			case VEHICLE_DETECT:
				ITS_VehicleDetect(dpPointVD);
				if(g_bIsSendRoadInfConnect)
				{
					SEM_post( &sem_SendRoadInfReady );//通知发送数据信号量
				}
				break;
			//队长检测
			case QUEUE_DETECT:
				ITS_QueueDetect(dpPointQD);
				break;
			//测试调试
			case TEST_MODE:
				break;
			default:
				break;
		}
		
	

   }//while
   
}//tsk

/************************************************************************************/
/**
 * \function 	VD_SemOut_IO函数
 * \brief    	接收车辆检测结果数据，触发IO口输出LED灯亮
 * \					
 * \			1、使用IO3、IO4两个口，分别对应两个线框。
 * \            2、IO使用需在main中初始化
**/
void VD_SemOut_IO(Uint8 *isC) {
		static Bool Iscarcome[2] = {FALSE, FALSE};
		//第一框
		switch(isC[0]){
			case 1:
				//激活IO3输出					
				Roseek_Set_Pio_Outvalue(2,1);//IO3,1为高电平、0为低电平
				Roseek_SetISOOut_AsSwitch_State(0, 1);//ISO-OUT1,开启
				Iscarcome[0] = TRUE;
				break;
			case 0:
				Roseek_Set_Pio_Outvalue(2,0);//IO3,1为高电平、0为低电平
				Roseek_SetISOOut_AsSwitch_State(0, 0);//ISO-OUT1,关闭
				if(Iscarcome[0])
				{
					(g_carInfo.num[0])++;
					Iscarcome[0] = FALSE;
				}
				break;
			default:
				Roseek_Set_Pio_Outvalue(2,0);//IO3,1为高电平、0为低电平
				Roseek_SetISOOut_AsSwitch_State(0, 0);//ISO-OUT1,关闭
				break;
		}
		//第二框
		switch(isC[1]){
			case 1:
				//激活IO4输出					
				Roseek_Set_Pio_Outvalue(3,1);//IO4,1为高电平、0为低电平
				Iscarcome[1] = TRUE;
				break;
			case 0:
				Roseek_Set_Pio_Outvalue(3,0);//IO4,1为高电平、0为低电平
				if(Iscarcome[1])
				{
					(g_carInfo.num[1])++;
					Iscarcome[1] = FALSE;
				}
				break;
			default:
				Roseek_Set_Pio_Outvalue(3,0);//IO4,1为高电平、0为低电平
				break;
		}	
}
	
/************************************************************************************/
/**
 * \function 	PointDataConfig函数为图像处理算法虚拟线框Point参数初始化函数。
 * \brief    	虚拟线框Point点初始化函数，在算法中使用。
 * \					
 * \			1、以框心为中心，点0对应西北方向，点1对应西南，2对应东北，3对应东南
 * \            2、两个框
**/
void PointDataConfig(DpPoint *pt, int *pointdata) {
	
	int ptTmp[2][4][2];
	int i,j;
	//读取全局PointData中的点坐标，PointData在系统初始化时从Flash中读取。
	for(i = 0; i < 2; i++)
		for(j = 0; j < 4; j++)
		{
			ptTmp[i][j][0] = pointdata[8 * i + 2 * j];//(x , y)坐标x
			ptTmp[i][j][1] = pointdata[8 * i + 2 * j + 1];//(x , y)坐标y
		}
	//计算Point参数
	for(i = 0; i <2; i++)
	{
		(pt + i)->pt5x = (ptTmp[i][0][0] + ptTmp[i][2][0]) / 2;
		(pt + i)->pt5y = (ptTmp[i][0][1] + ptTmp[i][2][1]) / 2;
		(pt + i)->pt6x = (ptTmp[i][1][0] + ptTmp[i][3][0]) / 2;
		(pt + i)->pt6y = (ptTmp[i][1][1] + ptTmp[i][3][1]) / 2;

		(pt + i)->k1 = ((float) (ptTmp[i][1][1] - ptTmp[i][0][1])) / (ptTmp[i][1][0]
				- ptTmp[i][0][0] == 0 ? 1 : ptTmp[i][1][0] - ptTmp[i][0][0]);
		(pt + i)->k2 = ((float) (ptTmp[i][3][1] - ptTmp[i][2][1])) / (ptTmp[i][3][0]
				- ptTmp[i][2][0] == 0 ? 1 : ptTmp[i][3][0] - ptTmp[i][2][0]);
		(pt + i)->k3 = ((float) ((pt + i)->pt6y - (pt + i)->pt5y)) / ((pt + i)->pt6x - (pt + i)->pt5x);
		(pt + i)->k4 = ((float) (ptTmp[i][2][1] - ptTmp[i][0][1])) / (ptTmp[i][2][0]
				- ptTmp[i][0][0] == 0 ? 1 : ptTmp[i][2][0] - ptTmp[i][0][0]);
		(pt + i)->k5 = ((float) (ptTmp[i][3][1] - ptTmp[i][1][1])) / (ptTmp[i][3][0]
				- ptTmp[i][1][0] == 0 ? 1 : ptTmp[i][3][0] - ptTmp[i][1][0]);
		(pt + i)->b1 = (ptTmp[i][1][1] - (pt + i)->k1 * ptTmp[i][1][0]);
		(pt + i)->b2 = (ptTmp[i][3][1] - (pt + i)->k2 * ptTmp[i][3][0]);
		(pt + i)->b3 = ((pt + i)->pt6y - (pt + i)->k3 * (pt + i)->pt6x);
		(pt + i)->b4 = (ptTmp[i][2][1] - (pt + i)->k4 * ptTmp[i][2][0]);
		(pt + i)->b5 = (ptTmp[i][3][1] - (pt + i)->k5 * ptTmp[i][3][0]);

		(pt + i)->xMax = 0;
		(pt + i)->yMax = 0;
		(pt + i)->xMin = 99999;
		(pt + i)->yMin = 99999;
		for (j = 0; j < 4; j++) {
			if ((pt + i)->xMax < ptTmp[i][j][0])
				(pt + i)->xMax = ptTmp[i][j][0];
			if ((pt + i)->yMax < ptTmp[i][j][1])
				(pt + i)->yMax = ptTmp[i][j][1];
			if ((pt + i)->xMin > ptTmp[i][j][0])
				(pt + i)->xMin = ptTmp[i][j][0];
			if ((pt + i)->yMin > ptTmp[i][j][1])
				(pt + i)->yMin = ptTmp[i][j][1];
		}
	}
}

/************************************************************************************/
/**
 * \function 	GetCfgDataFromFlash()函数为从FLASH中读取配置（与配置工具通信获取）。
 * \brief    	从Flash中读取原始配置数据，并进行数据解析。
 * \					
 * \			1、虚拟线框配置数据(分VD和QD两个线框)
 * \            2、车道编号配置数据
**/

void ReadPointDataFromFlash(){
	int 	i,j,k,n;
	char 	background[2][80];
	char    tmpNum[3] = {0,0,0};
	//从FLASH中读取虚拟线框配置
	Roseek_Flash_Read((Uint8 *)background[0],CFGVDPOINTADD,80);//车辆检测VD框
	Roseek_Flash_Read((Uint8 *)background[1],CFGQDPOINTADD,80);//队长检测QD框
	//数据解析
	j=0;
	k=0;
	for(i=0;i<2;i++)
	{
		for(n=0;n<80;n++){
			if(background[i][n]>='0'&&background[i][n]<='9'){//数字转存
			 	tmpNum[j]=10*tmpNum[j]+(background[i][n] - '0');//char转整型数
			 	++j;
			}
			if(background[i][n]==' '||background[i][n]=='\r'){
				switch(j){
					case 1:
						if(i == 0)
							PointDataVD[k]=tmpNum[0];
						else if(i == 1)
							PointDataQD[k]=tmpNum[0];
						break;
					case 2:
						if(i == 0)
							PointDataVD[k]=10*tmpNum[0]+tmpNum[1];
						else if(i == 1)
							PointDataQD[k]=10*tmpNum[0]+tmpNum[1];
						break;
					case 3:
						if(i == 0)
							PointDataVD[k]=100*tmpNum[0]+10*tmpNum[1]+tmpNum[2];
						else if(i == 1)
							PointDataQD[k]=100*tmpNum[0]+10*tmpNum[1]+tmpNum[2];
						break;
					default:
						break;
				}
				//临时量归零
				j=0;
				tmpNum[0]=0;
				tmpNum[1]=0;
				tmpNum[2]=0;
				++k;
			}
			if(k==16){//16个坐标数据读取完毕
				k=0;
				break;
			}
		}
	}

}
/**
 * \function 	ReadSetInfFormFlash()函数为从FLASH中读取相机配置信息。
 * \brief    	从Flash中读取原始配置数据，并转存至当前状态结构体。
 * \					
 * \			1、Flash起始地址CFGCOMMONADD存储当前运行状态
 * \            2、Flash起始地址CFGVDADD存储车辆检测配置，Flash起始地址CFGQDADD存储队长检测配置
 * \update      13.12.31修正BUG
**/
void ReadSetInfFormFlash(Bool isReadflash)
{
	static int ee3mode = 0;
	char scfgtmpVD[30];
	char scfgtmpQD[30];
	char Sdata[80];
	char word[3][30];
	int n1 = 3;
	int n2 = 2;
	Bool decodefail[3] = {FALSE, FALSE, FALSE};//解析配置标记

	if(isReadflash)
	{	
		//从Flash中读配置数据
		Roseek_Flash_Read((Uint8 *)Sdata, CFGCOMMONADD, 80);
		Roseek_Flash_Read((Uint8 *)scfgtmpVD, CFGVDADD, 30);
		Roseek_Flash_Read((Uint8 *)scfgtmpQD, CFGQDADD, 30);
		
		ee3mode = (int)(Sdata[0] - '0');
		if(ee3mode == VEHICLE_DETECT || ee3mode == QUEUE_DETECT || ee3mode == TEST_MODE)
		{
			//解析配置数据
			g_EE3Cur.RunMode = ee3mode;
			//通用配置
			decodefail[0] = DecodingCfgData(Sdata, n1, word);
			if(decodefail[0] == TRUE)
			{
				strcpy(g_EE3Cur.CameraIp, word[0]);
				strcpy(g_EE3Cur.ServerIp, word[1]);
				g_EE3Cur.ServerPort = (Uint16)(StringConvertInt(word[2]));
			}
			//车辆检测配置
			decodefail[1] = DecodingCfgData(scfgtmpVD, n2, word);
			if(decodefail[1] == TRUE)
			{
				g_EE3Cur.UploadTime = StringConvertInt(word[0]);
				g_EE3Cur.NightTrafficStream = StringConvertInt(word[1]);
			}	
			//队长检测配置
			decodefail[2] = DecodingCfgData(scfgtmpQD, n2, word);	
			if(decodefail[2] == TRUE)
			{		
				g_EE3Cur.RoadNum[0] = StringConvertInt(word[0]);
				g_EE3Cur.RoadNum[1] = StringConvertInt(word[1]);
			}
			if((decodefail[0] && decodefail[1] && decodefail[2]) == TRUE)
			{
				g_EE3Cur.Isreadflash = 1;//读取配置成功
			}
			else
			{
				g_EE3Cur.Isreadflash = 0;//读配置失败
			}			
		}
		else
		{
			g_EE3Cur.Isreadflash = 0;//读配置失败	
		}
	}	
}
/**
 * \function 	DecodingCfgData()函数为从字符流中解析字符数据并转存。
 * \brief    	输入：参数*bufstream字符流（格式为\n Word \r）
 * \				  参数wordnum要解析的字段数
 * \			输出：参数word[][30]转存字符数据数组（格式为 Word \0）		
 * \			注意：
 * \update      13.12.31修正BUG
**/
Bool DecodingCfgData(char *bufstream, int wordnum, char word[][30])
{	
	int i;
	int nCountRow = 0;//已读取的字段数
	int nWord;//字符数计数
	Bool isWord = FALSE;//标记

	//转存至word数组中
	for(i = 0;;i++)
	{
		if(nCountRow >= wordnum)
		{
			break;
		}
		if(bufstream[i] == '\n')
		{
			nWord = 0;
			isWord = TRUE;
			continue;
		}
		if(bufstream[i] == '\r')
		{
			word[nCountRow][nWord] = '\0';//添加尾部标识
			nCountRow++;
			isWord = FALSE;
			continue;
		}
		//是否是正确的字符
		if(( '0' <= bufstream[i] && bufstream[i] <= '9' ) || bufstream[i] == '.')
		{
			if(isWord)
			{
				word[nCountRow][nWord] = bufstream[i];
				nWord++;
			}
		}
		else
		{
			return FALSE;
		}
		
	}
	return TRUE;
			
}

/**
 * \function 	StringConvertInt()将数字字符流转换为对应的int型变量。
 * \brief    	输入：*p字符流（格式例“1234 \0"）
 * \			输出：Intp（格式例 1234）
 * \			注意：1、以'\0'结尾；2、注意长度，int为32位
 * \
**/
int StringConvertInt(char *p)
{
	int i;
	int nWord = 0;
	int Intp = 0;
	int tmp[10];
	for(i = 0;;i++)
	{
		if(p[i] == '\0')
			break;
		tmp[i] = (int)(p[i] - '0');
		nWord++;		
	}
	for(i = 0;i<nWord;i++)
	{
		Intp += tmp[i] * pow(10, (nWord - i - 1));
	}
	return(Intp);
}
	


/************************************************************************************/
/**
 * \function 	ResetPointData
 * \brief    	每次系统启动时调用，线框配置数据重置。
 **/
void ResetPointData(){
	int i;
	for(i=0; i<16; i++)
		PointDataVD[i] = 0;
	for(i=0; i<16; i++)
		PointDataQD[i] = 0;

}


/************************************************************************************/
/**
 * \function 	DrawPoint
 * \brief    	将虚拟线框在图像上标记,画矩形框
 * \					
 * \			1、以框心为中心，点0对应西北方向，点1对应西南，2对应东北，3对应东南
 * \            2、每个虚拟线框由四点确定，取四点所覆盖的最大矩形框
 * \            3、原始图像为YUV422格式
**/
//取4个变量的最大、最小值函数
void Sortminmax(int a,int b,int c,int d,int *min,int *max){
	int e[4];
	int i;
	*min=a;
	*max=a;
	e[0]=a;
	e[1]=b;
	e[2]=c;
	e[3]=d;
	for(i=0;i<4;i++){
		if(e[i]<*min)
			*min=e[i];
		if(e[i]>*max)
			*max=e[i];
	}
}
//画框函数
void DrawPoint(Uint8 *in_data, int *PointData){

		int ColMin,ColMax,RowMin,RowMax;
		int ptTmp[2][4][2];
		int i,j;
		//读取全局PointData中的点坐标，PointData在系统初始化时从Flash中读取。
		for(i = 0; i < 2; i++){
			for(j = 0; j < 4; j++)
			{
				ptTmp[i][j][0] = PointData[8 * i + 2 * j];//(x , y)坐标x
				ptTmp[i][j][1] = PointData[8 * i + 2 * j + 1];//(x , y)坐标y
			}
		}
		//画第一线框
		//取横坐标X的最大、最小值
		Sortminmax(ptTmp[0][0][0],ptTmp[0][1][0],ptTmp[0][2][0],ptTmp[0][3][0],&ColMin,&ColMax);
		//取纵坐标y的最大、最小值
		Sortminmax(ptTmp[0][0][1],ptTmp[0][1][1],ptTmp[0][2][1],ptTmp[0][3][1],&RowMin,&RowMax);
		for(i=(ALG_COL*RowMin+ColMin)*2;i<=(ALG_COL*RowMin+ColMax)*2;i=i+2){
		    *(in_data+i)=255;
			}
	    for(i=(ALG_COL*RowMax+ColMin)*2;i<=(ALG_COL*RowMax+ColMax)*2;i=i+2){
		    *(in_data+i)=255;
			}
		for(i=(ALG_COL*RowMin+ColMin)*2;i<=(ALG_COL*RowMax+ColMin)*2;i=i+2*ALG_COL){
		    *(in_data+i)=255;
			}
	    for(i=(ALG_COL*RowMin+ColMax)*2;i<=(ALG_COL*RowMax+ColMax)*2;i=i+2*ALG_COL){
		    *(in_data+i)=255;
			}
		//画第二线框
		//取横坐标X的最大、最小值
		Sortminmax(ptTmp[1][0][0],ptTmp[1][1][0],ptTmp[1][2][0],ptTmp[1][3][0],&ColMin,&ColMax);
		//取纵坐标y的最大、最小值
		Sortminmax(ptTmp[1][0][1],ptTmp[1][1][1],ptTmp[1][2][1],ptTmp[1][3][1],&RowMin,&RowMax);
		for(i=(ALG_COL*RowMin+ColMin)*2;i<=(ALG_COL*RowMin+ColMax)*2;i=i+2){
		    *(in_data+i)=255;
			}
	    for(i=(ALG_COL*RowMax+ColMin)*2;i<=(ALG_COL*RowMax+ColMax)*2;i=i+2){
		    *(in_data+i)=255;
			}
		for(i=(ALG_COL*RowMin+ColMin)*2;i<=(ALG_COL*RowMax+ColMin)*2;i=i+2*ALG_COL){
		    *(in_data+i)=255;
			}
	    for(i=(ALG_COL*RowMin+ColMax)*2;i<=(ALG_COL*RowMax+ColMax)*2;i=i+2*ALG_COL){
		    *(in_data+i)=255;
			}
	    //*(in_data+ALG_COL*P0y+P0x)=255;
	    //*(in_data+ALG_COL*P1y+P1x)=255;
	    //*(in_data+ALG_COL*P2y+P2x)=255;
	    //*(in_data+ALG_COL*P3y+P3x)=255;
}

/************************************************************************************/
/**
 * \function 	Count_RunTime
 * \brief    	计算程序段运行时间函数
 * \					
 * \			1、读取程序中的两个计时节点，算时间差
 * \            
**/
void Count_RunTime(Uint32 *nRT)
{
	static int 		nCountR = 0;
	static float    nSumRT = 0;
	if(nCountR < NUM_RUMTIME)
	{		
		RunTimeOnce[nCountR] = (abs(nRT[1] - nRT[0]))/1000;	//毫秒
		nSumRT = nSumRT + RunTimeOnce[nCountR];
		nCountR++;		
	}
	if(nCountR == NUM_RUMTIME)
	{
		//记录结果
		g_CmdUdata.nSumRT = nSumRT;
		g_CmdUdata.nFrame = NUM_RUMTIME / nSumRT * 1000;
		//SEM_post( &sem_DataRunTimeN );//激活上传数据
		nSumRT = 0;
		nCountR = 0;
	}
}
/*****************************************************************************
******************************车辆检测功能***********************************
*****************************************************************************/
void ITS_VehicleDetect(DpPoint *dpPVD)
{
	//车辆检测算法变量
	static Uint8 ptn = COIL_NUM;
	static Uint8 frame_n = FRAMEOFBACK_NUM;
	static Uint8 gsk = QUEUE_LENGTH;
	float *nMatp;
	Uint8   *pui8inFrameBufY;//当前前景缓存Y分量
	Uint8   *pui8BFrameBufYVD;//车辆检测背景图像缓存Y分量
	static Uint8 isCar[2];

	//车辆检测参数初始化
	nMatp = (float *)malloc(sizeof(float) * ( gsk * 2 + 1));
	memset(nMatp, 0, sizeof(float) * ( gsk * 2 + 1));		
	GaussianPram(nMatp, gsk);
	pui8inFrameBufY = g_ImgBufofFrontS.pui8YCrCb_YBuf;//当前前景的Y分量
	pui8BFrameBufYVD = g_ImgBufofBackVD.pui8YCrCb_YBuf;//背景Y分量
    
    //车辆检测函数
	GrayscaleVD(pui8inFrameBufY, g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd, dpPVD,
		&ptn, isCar, nMatp, pui8BFrameBufYVD, frame_n);
	//IO输出结果
	VD_SemOut_IO(isCar);					
	free(nMatp);
	nRunT[1] = Roseek_GetTickCount();//终止计数
	//时间统计
	Count_RunTime(nRunT);	
}

/*****************************************************************************
***************************队列长度检测功能**********************************
*****************************************************************************/
void ITS_QueueDetect(DpPoint *dpPQD)
{
	Uint8   *pui8inFrameBufY;//当前前景缓存Y分量
	Uint8   *pui8BFrameBufY1;//队列检测背景图像缓存Y分量
	Uint8   *pui8BFrameBufY2;
	static  Bool    bDpSem1 = TRUE;           //标志
	static  Bool    bDpSem2 = TRUE;
	Uint32    ui8Firstip1;
	Uint32    ui8Firstip2;
	Uint32    ui8Lastip1;
	Uint32    ui8Lastip2;
	Uint32    ui8RoadData1;
	Uint32    ui8RoadData2;

	pui8inFrameBufY = g_ImgBufofFrontS.pui8YCrCb_YBuf;//当前前景的Y分量
	
	//第一路状态检测
	if(g_AmcdSem1.bStart){
	      switch(g_AmcdSem1.comm){

			    //"红转绿"状态,激活背景建模
	            case NETINFORTG: 
	            	pui8BFrameBufY1 = g_ImgBufofBackS.pui8YCrCb_YBuf;//背景建模获得的背景图像Y分量           	          
		            //调用背景建模算法
                    gaussian(pui8BFrameBufY1, pui8inFrameBufY, 
                    	g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd, 
                    	dpPQD->yMin, dpPQD->yMax, dpPQD->xMin, dpPQD->xMax );
                    break;

			 	//控制端请求"队列检测"，发送结果
		      	case NETREQUEST: 
		      		while(bDpSem1){//一个周期仅执行一次               		               					  
		                  //调用队长检测算法
                          GrayscaleQD(pui8BFrameBufY1, pui8inFrameBufY,
                          		dpPQD, &ui8Firstip1, &ui8Lastip1 );
		                  //队长结果生成
						  ui8RoadData1=ui8Firstip1-ui8Lastip1;
						  //容错判断
						  if(ui8RoadData1>0&&ui8RoadData1<999){
                          		g_BpOutputData1.data=abs(ui8RoadData1);//队长检测峁?
								g_BpOutputData1.agree=1;
						  }
						  else{
								g_BpOutputData1.data=0;
								g_BpOutputData1.agree=0;
						  }
						  g_BpOutputData1.cameraN=g_EE3Cur.RoadNum[0];//车道号
                          g_BpOutputData1.comm=NETRETURN; //返回命令    			 			   
		  				  bDpSem1=FALSE;
		  			}
		  			//激活队列检测结果数据上传
		  			SEM_post( &sem_DpDataReady1 ); 								
		  			//激活JPEG编码后存入SD卡线程
					//SEM_post( &sem_JpegtoSDReady ); 
					//激活IO5输出					
					Roseek_Set_Pio_Outvalue(4,1);//IO5,1为高电平、0为低电平 					
					//Roseek_SetISOOut_AsSwitch_State( 0, 1);//ISO-OUT1,开启
					TSK_sleep( 2 );            		                                    
                    break; 

				//"绿转红"状态，则等待，队列检测准备,执行次数清零
	         	case NETINFOGTR:
					bDpSem1=TRUE;
					Roseek_Set_Pio_Outvalue(4,0);//IO4,1为高电平、0为低电平
					//Roseek_SetISOOut_AsSwitch_State( 0, 0);//ISO-OUT1,关闭
					TSK_sleep( 2 );
		        	break;

	         	default:
	            	break;
	   	  }
		  
	    }

		//第二路状态检测
		if(g_AmcdSem2.bStart){
	        switch(g_AmcdSem2.comm){

				//"红转绿"状态,激活背景建模
	            case NETINFORTG:
	            	pui8BFrameBufY2 = g_ImgBufofBackS2.pui8YCrCb_YBuf;//背景建模获得的背景图像Y分量    	          		             	
		            //调用背景建模算法
                    gaussian(pui8BFrameBufY2, pui8inFrameBufY, 
                    	g_bpalgData2.ww, g_bpalgData2.mean, g_bpalgData2.sd, 
                    	(dpPQD + 1)->yMin, (dpPQD + 1)->yMax, (dpPQD + 1)->xMin, (dpPQD + 1)->xMax );
                    break;

			  	//控制端请求"队列检测"
		      	case NETREQUEST:
		      		while(bDpSem2){//一个周期仅执行一次    		                 						  
		                  //调用队列检测算法
                          GrayscaleQD(pui8BFrameBufY2, pui8inFrameBufY,
                          		(dpPQD + 1), &ui8Firstip2, &ui8Lastip2 );
						  
		                  //结果生成并作容错判断
						  ui8RoadData2=ui8Firstip2-ui8Lastip2;
						  if(ui8RoadData2>0&&ui8RoadData2<999){
                          		g_BpOutputData2.data=abs(ui8RoadData2);//队长检测结果
								g_BpOutputData2.agree=1;
						  }
						  else{
								g_BpOutputData2.data=0;
								g_BpOutputData2.agree=0;
						  }
						  g_BpOutputData2.cameraN=g_EE3Cur.RoadNum[1];//车道号			  
                          g_BpOutputData2.comm=NETRETURN; //返回命令		  			 			   
		  				  bDpSem2=FALSE;
		  			}
		  			//激活队列检测结果数据上传
		  			SEM_post( &sem_DpDataReady2 ); 
		  			//激活JPEG编码后存入SD卡线程
		  			//SEM_post( &sem_JpegtoSDReady ); 
		  			//激活IO6输出
					Roseek_Set_Pio_Outvalue(5,1);//IO6,1为高电平、0为低电平 
					//Roseek_SetISOOut_AsSwitch_State( 1, 1);//ISO-OUT2,开启
					TSK_sleep( 2 );                                                        		                  
                    break; 

				//"绿转红"状态，则等待，队列检测准备,执行次数清零
	         	case NETINFOGTR:
					bDpSem2=TRUE;
					Roseek_Set_Pio_Outvalue(5,0);//IO6,1为高电平、0为低电平
					//Roseek_SetISOOut_AsSwitch_State( 1, 0);//ISO-OUT2,关闭
					TSK_sleep( 2 );
		       		break;

	         	default:
	           		break;
	     	}

	  	}
}


/************************************************************************************/
/**
 * \function 	Test_PointDPlus
 * \brief    	手动画框函数，参考文档点的布局
 * \					
 * \			
 * \            
**/
void Test_PointDPlus()
{
	static int step = 60;
	//线框的点坐标
	PointDataVD[0] = 0;
	PointDataVD[1] = 0;
	PointDataVD[2] = 0;	
	PointDataVD[3] = step;
	PointDataVD[4] = ALG_COL;
	PointDataVD[5] = 0;
	PointDataVD[6] = ALG_COL;	
	PointDataVD[7] = step;

}

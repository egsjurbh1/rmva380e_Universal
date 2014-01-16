/**
 * \file      	ITS_ImgFunc.c 
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.1.0
 * \date      	2014.1.15
 * \brief     	图像处理线程用到的一些函数
 * \update      
**/
/************************************************************************************/
#include "EE3_common.h"
#include "EE3_Imgfunc.h"
/************************************************************************************/
/**
 * \function 	PointDataConfig函数为图像处理算法虚拟线框Point参数初始化函数。
 * \brief    	虚拟线框Point点初始化函数，在算法中使用。
 * \					
 * \			1、以框心为中心，点0对应西北方向，点1对应西南，2对应东北，3对应东南
 * \            2、两个框
**/
void PointDataConfig(DpPoint *pt, int *pointdata) 
{
	
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
 * \function 	ReadPointDataFromFlash()函数为从FLASH中读取虚拟线框配置（与配置工具通信获取）。
 * \brief    	从Flash中读取原始配置数据，并进行数据解析。
 * \					
 * \			1、虚拟线框配置数据(分VD和QD两个线框)
 * \            2、车道编号配置数据
 * \update		增加读取判断2014.1.15
**/

void ReadPointDataFromFlash()
{
	int 	i,j,k,n;
	char 	background[2][81];
	char    tmpNum[3] = {0,0,0};
	//从FLASH中读取虚拟线框配置
	Roseek_Flash_Read((Uint8 *)background[0],CFGVDPOINTADD,80);//车辆检测VD框
	Roseek_Flash_Read((Uint8 *)background[1],CFGQDPOINTADD,80);//队长检测QD框
	//数据解析
	j=0;
	k=0;
	for(i=0;i<2;i++)
	{
		if(background[i][79] != 0x55)
			continue;
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
/************************************************************************************/
/**
 * \function 	StringConvertInt()将数字字符流转换为对应的int型变量。
 * \brief    	输入：*p字符流（格式例“1234 \0"）
 * \			输出：Intp（格式例 1234）
 * \			注意：1、以'\0'结尾；2、注意长度，int为32位
 * \
**/
static int StringConvertInt(char *p)
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
 * \function 	DecodingCfgData()函数为从字符流中解析字符数据并转存。
 * \brief    	输入：参数*bufstream字符流（格式为\n Word \r）
 * \				  参数wordnum要解析的字段数
 * \			输出：参数word[][30]转存字符数据数组（格式为 Word \0）		
 * \			注意：
 * \update      13.12.31修正BUG
**/
static Bool DecodingCfgData(char *bufstream, int wordnum, char word[][30])
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
/************************************************************************************/
/**
 * \function 	ReadSetInfFormFlash()函数为从FLASH中读取相机配置信息。
 * \brief    	从Flash中读取原始配置数据，并转存至当前状态结构体。
 * \					
 * \			1、Flash起始地址CFGCOMMONADD存储当前运行状态
 * \            2、Flash起始地址CFGVDADD存储车辆检测配置，Flash起始地址CFGQDADD存储队长检测配置
 * \update		增加读取判断2014.1.15
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
			if(Sdata[79] == 0x55)//已写入标记判断
			{
				decodefail[0] = DecodingCfgData(Sdata, n1, word);
				if(decodefail[0] == TRUE)
				{
					strcpy(g_EE3Cur.CameraIp, word[0]);
					strcpy(g_EE3Cur.ServerIp, word[1]);
					g_EE3Cur.ServerPort = (Uint16)(StringConvertInt(word[2]));
				}
			}
			if(scfgtmpVD[29] == 0x55)//已写入标记判断
			{
				decodefail[1] = DecodingCfgData(scfgtmpVD, n2, word);
				if(decodefail[1] == TRUE)
				{
					g_EE3Cur.UploadTime = StringConvertInt(word[0]);
					g_EE3Cur.NightTrafficStream = StringConvertInt(word[1]);
				}
			}
			//车辆检测配置
			if(scfgtmpQD[29] == 0x55)//已写入标记判断
			{	
			//队长检测配置
				decodefail[2] = DecodingCfgData(scfgtmpQD, n2, word);	
				if(decodefail[2] == TRUE)
				{		
					g_EE3Cur.RoadNum[0] = StringConvertInt(word[0]);
					g_EE3Cur.RoadNum[1] = StringConvertInt(word[1]);
				}
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
static void Sortminmax(int a,int b,int c,int d,int *min,int *max)
{
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
void DrawPoint(Uint8 *in_data, int *PointData)
{
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

/************************************************************************************/
/**
 * \function 	VD_SemOut_IO函数
 * \brief    	接收车辆检测结果数据，触发IO口输出LED灯亮
 * \					
 * \			1、使用IO3、IO4两个口，分别对应两个线框。
 * \            2、IO使用需在main中初始化
**/
void VD_SemOut_IO(Uint8 *isC) 
{
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
 * \function 	ResetPointData
 * \brief    	每次系统启动时调用，线框配置数据重置。
 **/
void ResetPointData()
{
	int i;
	for(i=0; i<16; i++)
		PointDataVD[i] = 0;
	for(i=0; i<16; i++)
		PointDataQD[i] = 0;

}

/************************************************************************************/
/**
 * \function 	SendSemCycle
 * \brief    	周期性发送信号量
 **/
void SendSemCycle()
{
	Uint8   RTC_time[8];
	static  Uint8 temp_min = 0;
	static  Uint8 temp_hour = 0;
	
	Roseek_RTC_ReadTime(RTC_time);//当前系统时间
	if(RTC_time[5]>temp_min)
	{
		if(RTC_time[5]-temp_min > CONNECT_CYCLE)
		{	
			temp_min = RTC_time[5];
			temp_hour = RTC_time[4];
			SEM_post( &sem_SendRoadInfReconnect );//通知连接服务器信号量
			
		}							
	}
	else
	{
		if( temp_min < (60 - CONNECT_CYCLE) || temp_hour != RTC_time[4])
		{	
			temp_min = RTC_time[5] ;
			temp_hour = RTC_time[4] ;
			SEM_post( &sem_SendRoadInfReconnect );//通知连接服务器信号量
			
		}
	}
}


//////////////////////////////////////////////////////////////////////
// File name: EE3_ImgMain.h      		                            //    
//////////////////////////////////////////////////////////////////////

/**
 * \file      	EE3_ImgMain.h
 * \author    	LQ
 * \version   	0.05
 * \date      	2012.12.1
 * \brief     	EE3_ImgMain中的全局变量。
 *              ！更新：
**/
#ifndef _EE3_IMGMAIN_H
#define _EE3_IMGMAIN_H

extern ITS_ImgBufStruct      g_ImgBufofBackS;
extern ITS_ImgBufStruct      g_ImgBufofBackS2;
extern ITS_ImgBufStruct      g_ImgBufofFrontS;
extern ITS_ImgBufStruct  	 g_ImgBufofBackVD;
extern RoseekImgBufStruct		g_ImgBufStruct[]; //图像原始数据缓存结构体
extern BpalgData g_bpalgData;
extern BpalgData g_bpalgData2;
extern AcmdSem 		g_AmcdSem1;
extern AcmdSem 		g_AmcdSem2;
extern PcInData     g_BpInputData;
extern PcInData 	g_BpOutputData1;
extern PcInData 	g_BpOutputData2;
extern volatile	    Bool	  g_bIsFcontoDecClientConnect;
extern volatile     Bool    g_bIsSendRoadInfConnect;
extern Uint32  nRunT[2];
extern volatile    float   runTimeT[60];
extern volatile Bool IsReadFlash;
extern EE3_CfgStruct 	g_EE3Cur;//相机运行状态
extern CmdUData 		g_CmdUdata;
//函数
extern void EE3_YUV2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );

#endif


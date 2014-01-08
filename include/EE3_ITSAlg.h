//////////////////////////////////////////////////////////////////////
// File name: EE3_ITSAlg.h      		                            //    
//////////////////////////////////////////////////////////////////////

/**
 * \file      	EE3_ITSAlg.h
 * \author    	LQ
 * \version   	0.0.1
 * \date      	2014.1.8
 * \brief     	智能交通图像处理算法。
 *				
 *              ！更新：
**/
#ifndef	_EE3_ITSALG_H
#define	_EE3_ITSALG_H

#define HEIGHT			544 //对应ALG_ROW
#define WIDTH 			960 //ALG_COL
#define SEG_THR 		25
#define PER_THR 		50.0 //数车阈值
#define THRESHOLD1 		30
#define BAC_READY 		10.0 
#define BG_THR			15  //建模阈值
/************************************************************************************/
#define THRESHOLD1 		30    //判断是前景还是噪声，此值也要斟酌使用，会滤掉一部分前景
#define THRESHOLD2 		0.08  //车道横向物体占有率，为了滤去非车物
#define THRESHOLD3 		20    //判定白点数之间间距使用
#define THRESHOLD4 		10    //车辆行数

//高斯背景建模
extern void gaussian(Uint8 *b_data, Uint8 *i_data, float *ww, float *mean, float *sd,int yMin, int yMax, int xMin, int xMax);
//高斯背景建模算法中所需的初始化函数
extern void setwwmeansd(Float *ww, Float *mean, Float *sd);
extern void GaussianPram(Float *nMat, int k);
//车辆检测
extern int GrayscaleVD(Uint8 *in_data, float *ww, float *mean, float *sd, DpPoint *ptP,
		 Uint8 *ptn, Uint8 *carNum,	float *nMat, Uint8 *back_data,	Uint8 numofframe);
//队长检测
extern int GrayscaleQD(Uint8 *back_data,Uint8 *in_data,DpPoint *pt,Uint32 *firstip,Uint32 *lastip);


#endif

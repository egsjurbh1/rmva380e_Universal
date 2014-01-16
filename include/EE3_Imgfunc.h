/**
 * \file      	EE3_Imgfunc.h
 * \author    	LQ
 * \version   	0.1.0
 * \date      	2014.1.16
 * \brief     	imgfunc函数库
 *              ！更新：
**/

#ifndef _EE3_IMGFUNC_H
#define _EE3_IMGFUNC_H

#define CONNECT_CYCLE 	5	//连接服务器周期，单位分钟
/***********************************************/
//**全局变量
extern int     PointDataVD[16];
extern int     PointDataQD[16];
extern EE3_CfgStruct g_EE3Cur;//相机运行状态
extern volatile     float  RunTimeOnce[NUM_RUMTIME];
extern CmdUData g_CmdUdata;
extern CarInfo g_carInfo;
/**********************************************/
//函数
//虚拟线框Point参数初始化函数
void PointDataConfig(DpPoint *pt, int *pointdata);
//从FLASH中读取虚拟线框配置
void ReadPointDataFromFlash();
//从FLASH中读取相机配置信息
void ReadSetInfFormFlash(Bool isReadflash);
//虚拟线框在图像上标记,画矩形框
void DrawPoint(Uint8 *in_data, int *PointData);
//手动画框函数
void Test_PointDPlus();
//计算程序段运行时间函数
void Count_RunTime(Uint32 *nRT);
//车辆检测结果数据，触发IO口输出
void VD_SemOut_IO(Uint8 *isC);
//线圈清零
void ResetPointData();
//周期性发送信号量
void SendSemCycle();

#endif



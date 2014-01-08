/**
 * \file      	ITS_ImgMain.c 
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.3.5
 * \date      	2014.1.8
 * \brief     	智能交通应用----图像处理主线程(车辆检测功能、队列长度检测功能)
 * \update      代码整理头文件
**/

#include "EE3_common.h"
#include "SDFS_filesys.h"
#include "EE3_ITSAlg.h"
#include "EE3_Imgfunc.h"
#include "EE3_Imgmain.h"
/**************************************************************************/
static void ITS_VehicleDetect(DpPoint *dpPVD);
static void ITS_QueueDetect(DpPoint  *dpPQD);
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


/*****************************************************************************
******************************车辆检测功能***********************************
*****************************************************************************/
static void ITS_VehicleDetect(DpPoint *dpPVD)
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
static void ITS_QueueDetect(DpPoint *dpPQD)
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




/**
 * \file      	ITS_ImgMain.c 
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.3.6
 * \date      	2014.1.15
 * \brief     	ÖÇÄÜ½»Í¨Ó¦ÓÃ----Í¼Ïñ´¦ÀíÖ÷Ïß³Ì(³µÁ¾¼ì²â¹¦ÄÜ¡¢¶ÓÁĞ³¤¶È¼ì²â¹¦ÄÜ)
 * \update      ¼ÓÈëºÍsendroadinfÏß³ÌÍ¬²½ĞÅºÅÂß¼­
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
 * \brief    	Í¼Ïñ´¦ÀíÏµÍ³Ïß³Ì
 * \			¸ÃÄ£¿é½ÓÊÜImgAcqÖĞĞÅºÅ´¥·¢£¬Ã¿Ö¡Í¼Ïñ´¥·¢		
 * \            1¡¢		
 * \            2¡¢
**/	
void tskImgmainFunc()
{
	 Uint8   *pui8IMGbuffer;//µ±Ç°²É¼¯Ö¡Í¼Ïñ»º´æ
	 Uint8	 i;
	 DpPoint dpPointVD[2];
	 DpPoint dpPointQD[2];	 
	 Uint8	 ui8tmp = 0;
	 Bool	 bBreak = FALSE;			//ÇĞ³ö±êÖ¾
	 Bool    bReadOnce = TRUE;
	 Bool    bsemSendtoDecready = FALSE;        
	 static	 Uint8	ui8FrameIndex = 0; //Ö¡»º´æĞòºÅ

	 //×¼±¸Ëã·¨ËùĞèÊı¾İ
	 setwwmeansd( g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd);
	 setwwmeansd( g_bpalgData2.ww, g_bpalgData2.mean, g_bpalgData2.sd);	
	 ResetPointData();//ÏßÈ¦ÇåÁã
	     
     while( 1 )
	 {
	    SEM_pend(&sem_AcmdBDReady,SYS_FOREVER);
		TSK_sleep( 2 );
				
		nRunT[0] = Roseek_GetTickCount();//¿ªÊ¼¼ÆÊı
		
		//******¶ÁÈ¡FLASHÖĞµÄÅäÖÃ²ÎÊı£¬Æô¶¯ºó¶ÁÈ¡Ò»´Î¡£
		if(bReadOnce)
		{
			ReadPointDataFromFlash();//¶ÁÏß¿òÅäÖÃÊı¾İ
			PointDataConfig(dpPointVD, PointDataVD);//³µÁ¾¼ì²âµÄÏß¿òÅäÖÃ
			PointDataConfig(dpPointQD, PointDataQD);//¶ÓÁĞ¼ì²âµÄÏß¿òÅäÖÃ
			ReadSetInfFormFlash(IsReadFlash);//¶ÁÏà»úÔËĞĞ²ÎÊıÊı¾İ
			//Test_PointDPlus();//²âÊÔÓÃ£¬ÊÖ¶¯Éè¶¨Ïß¿ò´óĞ¡
			bReadOnce=FALSE;
		}
			
		//ÅĞ¶Ïµ±Ç°´¦ÀíÄÄ¸öÔ­Ê¼Í¼Ïñ»º´æ
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
		g_ImgBufofFrontS.pui8FrameBuf = g_ImgBufStruct[ui8FrameIndex].pui8FrameBuf;//¸´ÖÆÇ°¾°Í¼Ïñ
		pui8IMGbuffer = g_ImgBufStruct[ui8FrameIndex].pui8FrameBuf;
		bsemSendtoDecready = g_bIsFcontoDecClientConnect; //·¢ËÍÍ¼ÏñÏß³Ì×¼±¸Íê±Ï
		//¼¤»îÏòÅäÖÃ¹¤¾ß´«ÊäÍ¼ÏñÏß³ÌĞÅºÅÁ¿
		if(bsemSendtoDecready)
		{
	    	SEM_post(&sem_FcontoDecReady);
		}
 		//´Ë´¦µ÷ÓÃ»­ĞéÄâÏß¿òº¯Êı		
		switch(g_EE3Cur.RunMode)
		{
			//³µÁ¾¼ì²â
			case VEHICLE_DETECT:
				DrawPoint(pui8IMGbuffer, PointDataVD);
				break;
			//¶Ó³¤¼ì²â
			case QUEUE_DETECT:
				DrawPoint(pui8IMGbuffer, PointDataQD);
				break;
			default:
				break;
		}
		//Çå³ı L2 Cache
		BCACHE_wbAll();

		//******×ª»»Ô­Ê¼Í¼ÏñYUV422ÖÁYUV420¸ñÊ½£¬²¢ÌáÈ¡Y·ÖÁ¿
		EE3_YUV2YCrCb420(	pui8IMGbuffer,
							g_ImgBufofFrontS.pui8YCrCb_YBuf, g_ImgBufofFrontS.pui8YCrCb_CrBuf, g_ImgBufofFrontS.pui8YCrCb_CbBuf,
							ALG_COL, ALG_ROW );
		
		g_ImgBufStruct[ui8FrameIndex].bProcessLocked = FALSE;//Ô­Ê¼Í¼Ïñ½âËø	

		//¹¦ÄÜÅĞ¶Ï
		switch(g_EE3Cur.RunMode)
		{
			//³µÁ¾¼ì²â
			case VEHICLE_DETECT:
				ITS_VehicleDetect(dpPointVD);
				if(g_bIsSendRoadInfConnect)
				{
					SEM_post( &sem_SendRoadInfReady );//Í¨Öª·¢ËÍÊı¾İĞÅºÅÁ¿
				}
				else
				{					
					SendSemCycle();//Ã¿¸ôÒ»¶ÎÊ±¼ä·¢Á¬½ÓĞÅºÅ
				}
				break;
			//¶Ó³¤¼ì²â
			case QUEUE_DETECT:
				ITS_QueueDetect(dpPointQD);
				break;
			//²âÊÔµ÷ÊÔ
			case TEST_MODE:
				break;
			default:
				break;
		}	

   }//while
   
}//tsk


/*****************************************************************************
******************************³µÁ¾¼ì²â¹¦ÄÜ***********************************
*****************************************************************************/
static void ITS_VehicleDetect(DpPoint *dpPVD)
{
	//³µÁ¾¼ì²âËã·¨±äÁ¿
	static Uint8 ptn = COIL_NUM;
	static Uint8 frame_n = FRAMEOFBACK_NUM;
	static Uint8 gsk = QUEUE_LENGTH;
	float *nMatp;
	Uint8   *pui8inFrameBufY;//µ±Ç°Ç°¾°»º´æY·ÖÁ¿
	Uint8   *pui8BFrameBufYVD;//³µÁ¾¼ì²â±³¾°Í¼Ïñ»º´æY·ÖÁ¿
	static Uint8 isCar[2];

	//³µÁ¾¼ì²â²ÎÊı³õÊ¼»¯
	nMatp = (float *)malloc(sizeof(float) * ( gsk * 2 + 1));
	memset(nMatp, 0, sizeof(float) * ( gsk * 2 + 1));		
	GaussianPram(nMatp, gsk);
	pui8inFrameBufY = g_ImgBufofFrontS.pui8YCrCb_YBuf;//µ±Ç°Ç°¾°µÄY·ÖÁ¿
	pui8BFrameBufYVD = g_ImgBufofBackVD.pui8YCrCb_YBuf;//±³¾°Y·ÖÁ¿
    
    //³µÁ¾¼ì²âº¯Êı
	GrayscaleVD(pui8inFrameBufY, g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd, dpPVD,
		&ptn, isCar, nMatp, pui8BFrameBufYVD, frame_n);
	//IOÊä³ö½á¹û
	VD_SemOut_IO(isCar);					
	free(nMatp);
	nRunT[1] = Roseek_GetTickCount();//ÖÕÖ¹¼ÆÊı
	//Ê±¼äÍ³¼Æ
	Count_RunTime(nRunT);	
}

/*****************************************************************************
***************************¶ÓÁĞ³¤¶È¼ì²â¹¦ÄÜ**********************************
*****************************************************************************/
static void ITS_QueueDetect(DpPoint *dpPQD)
{
	Uint8   *pui8inFrameBufY;//µ±Ç°Ç°¾°»º´æY·ÖÁ¿
	Uint8   *pui8BFrameBufY1;//¶ÓÁĞ¼ì²â±³¾°Í¼Ïñ»º´æY·ÖÁ¿
	Uint8   *pui8BFrameBufY2;
	static  Bool    bDpSem1 = TRUE;           //±êÖ¾
	static  Bool    bDpSem2 = TRUE;
	Uint32    ui8Firstip1;
	Uint32    ui8Firstip2;
	Uint32    ui8Lastip1;
	Uint32    ui8Lastip2;
	Uint32    ui8RoadData1;
	Uint32    ui8RoadData2;

	pui8inFrameBufY = g_ImgBufofFrontS.pui8YCrCb_YBuf;//µ±Ç°Ç°¾°µÄY·ÖÁ¿
	
	//µÚÒ»Â·×´Ì¬¼ì²â
	if(g_AmcdSem1.bStart){
	      switch(g_AmcdSem1.comm){

			    //"ºì×ªÂÌ"×´Ì¬,¼¤»î±³¾°½¨Ä£
	            case NETINFORTG: 
	            	pui8BFrameBufY1 = g_ImgBufofBackS.pui8YCrCb_YBuf;//±³¾°½¨Ä£»ñµÃµÄ±³¾°Í¼ÏñY·ÖÁ¿           	          
		            //µ÷ÓÃ±³¾°½¨Ä£Ëã·¨
                    gaussian(pui8BFrameBufY1, pui8inFrameBufY, 
                    	g_bpalgData.ww, g_bpalgData.mean, g_bpalgData.sd, 
                    	dpPQD->yMin, dpPQD->yMax, dpPQD->xMin, dpPQD->xMax );
                    break;

			 	//¿ØÖÆ¶ËÇëÇó"¶ÓÁĞ¼ì²â"£¬·¢ËÍ½á¹û
		      	case NETREQUEST: 
		      		while(bDpSem1){//Ò»¸öÖÜÆÚ½öÖ´ĞĞÒ»´Î               		               					  
		                  //µ÷ÓÃ¶Ó³¤¼ì²âËã·¨
                          GrayscaleQD(pui8BFrameBufY1, pui8inFrameBufY,
                          		dpPQD, &ui8Firstip1, &ui8Lastip1 );
		                  //¶Ó³¤½á¹ûÉú³É
						  ui8RoadData1=ui8Firstip1-ui8Lastip1;
						  //Èİ´íÅĞ¶Ï
						  if(ui8RoadData1>0&&ui8RoadData1<999){
                          		g_BpOutputData1.data=abs(ui8RoadData1);//¶Ó³¤¼ì²âá¹?
								g_BpOutputData1.agree=1;
						  }
						  else{
								g_BpOutputData1.data=0;
								g_BpOutputData1.agree=0;
						  }
						  g_BpOutputData1.cameraN=g_EE3Cur.RoadNum[0];//³µµÀºÅ
                          g_BpOutputData1.comm=NETRETURN; //·µ»ØÃüÁî    			 			   
		  				  bDpSem1=FALSE;
		  			}
		  			//¼¤»î¶ÓÁĞ¼ì²â½á¹ûÊı¾İÉÏ´«
		  			SEM_post( &sem_DpDataReady1 ); 								
		  			//¼¤»îJPEG±àÂëºó´æÈëSD¿¨Ïß³Ì
					//SEM_post( &sem_JpegtoSDReady ); 
					//¼¤»îIO5Êä³ö					
					Roseek_Set_Pio_Outvalue(4,1);//IO5,1Îª¸ßµçÆ½¡¢0ÎªµÍµçÆ½ 					
					//Roseek_SetISOOut_AsSwitch_State( 0, 1);//ISO-OUT1,¿ªÆô
					TSK_sleep( 2 );            		                                    
                    break; 

				//"ÂÌ×ªºì"×´Ì¬£¬ÔòµÈ´ı£¬¶ÓÁĞ¼ì²â×¼±¸,Ö´ĞĞ´ÎÊıÇåÁã
	         	case NETINFOGTR:
					bDpSem1=TRUE;
					Roseek_Set_Pio_Outvalue(4,0);//IO4,1Îª¸ßµçÆ½¡¢0ÎªµÍµçÆ½
					//Roseek_SetISOOut_AsSwitch_State( 0, 0);//ISO-OUT1,¹Ø±Õ
					TSK_sleep( 2 );
		        	break;

	         	default:
	            	break;
	   	  }
		  
	    }

		//µÚ¶şÂ·×´Ì¬¼ì²â
		if(g_AmcdSem2.bStart){
	        switch(g_AmcdSem2.comm){

				//"ºì×ªÂÌ"×´Ì¬,¼¤»î±³¾°½¨Ä£
	            case NETINFORTG:
	            	pui8BFrameBufY2 = g_ImgBufofBackS2.pui8YCrCb_YBuf;//±³¾°½¨Ä£»ñµÃµÄ±³¾°Í¼ÏñY·ÖÁ¿    	          		             	
		            //µ÷ÓÃ±³¾°¨Ä£Ëã·¨
                    gaussian(pui8BFrameBufY2, pui8inFrameBufY, 
                    	g_bpalgData2.ww, g_bpalgData2.mean, g_bpalgData2.sd, 
                    	(dpPQD + 1)->yMin, (dpPQD + 1)->yMax, (dpPQD + 1)->xMin, (dpPQD + 1)->xMax );
                    break;

			  	//¿ØÖÆ¶ËÇëÇó"¶ÓÁĞ¼ì²â"
		      	case NETREQUEST:
		      		while(bDpSem2){//Ò»¸öÖÜÆÚ½öÖ´ĞĞÒ»´Î    		                 						  
		                  //µ÷ÓÃ¶ÓÁĞ¼ì²âËã·¨
                          GrayscaleQD(pui8BFrameBufY2, pui8inFrameBufY,
                          		(dpPQD + 1), &ui8Firstip2, &ui8Lastip2 );
						  
		                  //½á¹ûÉú³É²¢×÷Èİ´íÅĞ¶Ï
						  ui8RoadData2=ui8Firstip2-ui8Lastip2;
						  if(ui8RoadData2>0&&ui8RoadData2<999){
                          		g_BpOutputData2.data=abs(ui8RoadData2);//¶Ó³¤¼ì²â½á¹û
								g_BpOutputData2.agree=1;
						  }
						  else{
								g_BpOutputData2.data=0;
								g_BpOutputData2.agree=0;
						  }
						  g_BpOutputData2.cameraN=g_EE3Cur.RoadNum[1];//³µµÀºÅ			  
                          g_BpOutputData2.comm=NETRETURN; //·µ»ØÃüÁî		  			 			   
		  				  bDpSem2=FALSE;
		  			}
		  			//¼¤»î¶ÓÁĞ¼ì²â½á¹ûÊı¾İÉÏ´«
		  			SEM_post( &sem_DpDataReady2 ); 
		  			//¼¤»îJPEG±àÂëºó´æÈëSD¿¨Ïß³Ì
		  			//SEM_post( &sem_JpegtoSDReady ); 
		  			//¼¤»îIO6Êä³ö
					Roseek_Set_Pio_Outvalue(5,1);//IO6,1Îª¸ßµçÆ½¡¢0ÎªµÍµçÆ½ 
					//Roseek_SetISOOut_AsSwitch_State( 1, 1);//ISO-OUT2,¿ªÆô
					TSK_sleep( 2 );                                                        		                  
                    break; 

				//"ÂÌ×ªºì"×´Ì¬£¬ÔòµÈ´ı£¬¶ÓÁĞ¼ì²â×¼±¸,Ö´ĞĞ´ÎÊıÇåÁã
	         	case NETINFOGTR:
					bDpSem2=TRUE;
					Roseek_Set_Pio_Outvalue(5,0);//IO6,1Îª¸ßµçÆ½¡¢0ÎªµÍµçÆ½
					//Roseek_SetISOOut_AsSwitch_State( 1, 0);//ISO-OUT2,¹Ø±Õ
					TSK_sleep( 2 );
		       		break;

	         	default:
	           		break;
	     	}

	  	}
}




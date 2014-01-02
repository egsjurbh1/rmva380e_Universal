/**
 * \file      	EE3_JPGEncoder.c 
 * \author    	LQ
 * \version   	0.09
 * \date      	2013.6.30
 * \brief       JPEG编码模块
 *             !更新:适应Acmd改进
**/

#include "EE3_common.h"
#include "SDFS_filesys.h"
#define	DAT_RGB_ROW_SIZE	2
#define	DAT_YUV_ROW_SIZE	2
#define	DAT_BAYER_ROW_SIZE	2
#define	DAT_MAX_SIZE		FINE_COL*6
#define	DAT_YMAX_SIZE		FINE_COL*2
#define	DAT_CMAX_SIZE		FINE_COL/2
/**************************************************************************/
extern volatile	Bool			g_bIsTrgClientConnect;
extern volatile	Bool			g_bIsFconClientConnect;
extern volatile Bool            BIsTrgFrameofBack;
extern RoseekImgBufStruct		g_ImgBufStruct[];
extern RoseekJPGEncBufStruct	g_JpgEncBufStruct;
extern RoseekImgInfStruct		g_ImgInfStruct;
extern ITS_ImgBufStruct      	g_ImgBufofFrontS;
extern DAT_HANDLE				g_hDAT4;
extern DAT_HANDLE				g_hDAT5;
extern DAT_HANDLE				g_hDAT6;
extern DAT_HANDLE				g_hDAT7;
/**************************************************************************/
#pragma	DATA_SECTION( org_ping, ".user_data_int" );
#pragma	DATA_ALIGN( org_ping, 4 );
Uint8 org_ping[DAT_MAX_SIZE];
#pragma	DATA_SECTION( org_pong, ".user_data_int" );
#pragma	DATA_ALIGN( org_pong, 4 );
Uint8 org_pong[DAT_MAX_SIZE];
#pragma	DATA_SECTION( y_ping, ".user_data_int" );
#pragma	DATA_ALIGN( y_ping, 4 );
Uint8 y_ping[DAT_YMAX_SIZE];
#pragma	DATA_SECTION( cr_ping, ".user_data_int" );
#pragma	DATA_ALIGN( cr_ping, 4 );
Uint8 cr_ping[DAT_CMAX_SIZE];
#pragma	DATA_SECTION( cb_ping, ".user_data_int" );
#pragma	DATA_ALIGN( cb_ping, 4 );
Uint8 cb_ping[DAT_CMAX_SIZE];
#pragma	DATA_SECTION( y_pong, ".user_data_int" );
#pragma	DATA_ALIGN( y_pong, 4 );
Uint8 y_pong[DAT_YMAX_SIZE];
#pragma	DATA_SECTION( cr_pong, ".user_data_int" );
#pragma	DATA_ALIGN( cr_pong, 4 );
Uint8 cr_pong[DAT_CMAX_SIZE];
#pragma	DATA_SECTION( cb_pong, ".user_data_int" );
#pragma	DATA_ALIGN( cb_pong, 4 );
Uint8 cb_pong[DAT_CMAX_SIZE];
/**************************************************************************/
extern void Roseek_Get_YCrCb420ForJPGEncoder( enum Roseek_ImgAcquisition_Format, Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
extern void Roseek_RGB2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
extern void Roseek_YUV2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
extern void Roseek_Bayer2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32, Uint32 );
void		EE3_YCrCb_Extraction( enum Roseek_ImgAcquisition_Format, Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
void 		EE3_Bayer2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
void 		EE3_RGB2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
void 		EE3_YUV2YCrCb420( Uint8*, Uint8*, Uint8*, Uint8*, Uint32, Uint32 );
/**************************************************************************/

/**
 * \function 	tskEncodeFunc
 * \brief    	JPEG编码模块
 * \			该模块作为线程存在于系统之中，用来将已经放置到Buffer中的图像原始数据压缩成JPEG格式的图像数据		
 * \			1、
 * \            2、
**/	
void tskEncodeFunc( )
{
	Uint8	i;
	Uint8	*pui8JpgEncOutBuf[2];
	Uint8	*pui8InputBufs[3];
	Uint32	jpg_size, imgRowSize, imgColSize;
	Bool	bBreak = FALSE;			//切出标志
	static	Uint8	ui8FrameIndex=0;//帧缓存序号
	enum Roseek_ImgAcquisition_Format	enumImgFormat;
	static	Uint8	ui8JpgBufIndex=0;
	Uint8	ui8tmp	= 0;

	while( TRUE)
	{
			SEM_pend(&sem_JpegReady,SYS_FOREVER);
			TSK_sleep( 2 );
			imgRowSize = FINE_ROW>>1;
			imgColSize = FINE_COL>>1;
			//判断JPEG缓冲
			bBreak = FALSE;
			if(ui8JpgBufIndex==1){
				ui8tmp=0;
			}
			else{
				ui8tmp=ui8JpgBufIndex+1;
			}
			
			for( i=ui8tmp; i<ui8tmp+2; i++ ){
				if( i>1 ){
					if( !g_JpgEncBufStruct.bJpgImgBuf_Lock[i-2] ){
						ui8JpgBufIndex = i-2;
						bBreak = TRUE;
						break;
					}
				}
				else{//i<=5
					if( !g_JpgEncBufStruct.bJpgImgBuf_Lock[i] ){
						ui8JpgBufIndex = i;
						bBreak = TRUE;
						break;
					}
				}
			}
			if( !bBreak ){
				g_ImgBufStruct[ui8FrameIndex].bProcessLocked = FALSE;
				continue;
			}
			pui8JpgEncOutBuf[0] = (Uint8*)&jpg_size;
			pui8JpgEncOutBuf[1] = g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex] + 4;
			//清除 L2 Cache
			BCACHE_wbAll();
			//开始编码
			pui8InputBufs[0] = g_ImgBufofFrontS.pui8YCrCb_YBuf;
			pui8InputBufs[1] = g_ImgBufofFrontS.pui8YCrCb_CrBuf;
			pui8InputBufs[2] = g_ImgBufofFrontS.pui8YCrCb_CbBuf;
			Roseek_JPGENC_Encode( pui8JpgEncOutBuf, pui8InputBufs, XDM_YUV_420P, 90, imgColSize, imgRowSize );
			    
			//配置JPG图像尺寸,填充抓拍信息(仅在抓拍模式下填充)
			*(Uint32*)g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex] = jpg_size;
			g_JpgEncBufStruct.bJpgImgBuf_Lock[ui8JpgBufIndex] = TRUE;
				
			//激活各个模式图像的上传模块
			SEM_post( &sem_ConDataReady);
	}
}







/**
 * \function 	EE3_YCrCb_Extraction
 * \brief    	该函数用于将YUV422、Bayer、RGB格式的图像原始数据转换成YUV420格式的数据，并将转换完毕的数据存放在用户指定的Y、Cr、Cb三个缓存中，用于JPEG编码
 * \			该模块的工作过程调用了智能相机系统的DAT模块，并占用了DAT模块的4、5、6、7通道。仔细阅读该模块可帮助用户了解DAT模块的工作原理	
 * \			
**/

void EE3_YCrCb_Extraction( enum Roseek_ImgAcquisition_Format enumImgFormat, Uint8* pui8SurImg, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 ui32ColSize, Uint32 ui32RowSize )
{
	switch( enumImgFormat ){
		case RGB_ARRAY ://RGB Array
			EE3_RGB2YCrCb420(	pui8SurImg,
								pui8YBuf,
								pui8CrBuf,
								pui8CbBuf,
								ui32ColSize,
								ui32RowSize );
			break;
		case YUV_ARRAY ://YUV Array
			EE3_YUV2YCrCb420(	pui8SurImg,
								pui8YBuf,
								pui8CrBuf,
								pui8CbBuf,
								ui32ColSize,
								ui32RowSize );
			break;
		case BAYER_ARRAY ://Bayer Array
			EE3_Bayer2YCrCb420(	pui8SurImg,
								pui8YBuf,
								pui8CrBuf,
								pui8CbBuf,
								ui32ColSize,
								ui32RowSize );
			break;
		default:
			break;
	}
}


/**
 * \function 	EE3_RGB2YCrCb420
 * \brief    	该函数用于将RGB格式的图像原始数据转换成YUV420格式的数据，并将转换完毕的数据存放在用户指定的Y、Cr、Cb三个缓存中，用于JPEG编码
 * \			该模块的工作过程调用了智能相机系统的DAT模块，并占用了DAT模块的4、5、6、7通道。仔细阅读该模块可帮助用户了解DAT模块的工作原理	
 * \			
**/	
void EE3_RGB2YCrCb420( Uint8* pui8YUVInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 colsize, Uint32 rowsize)
{
	Uint32 i;
	Uint32 subrow = DAT_RGB_ROW_SIZE;
	Uint32 bsize = colsize*3*subrow;
	Uint32 loop = rowsize/subrow/2;
	Uint8* restrict pSrc = pui8YUVInBuf;
	Uint8* restrict pDstY = pui8YBuf;
	Uint8* restrict pDstCr = pui8CrBuf;
	Uint8* restrict pDstCb = pui8CbBuf;

	Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
	for( i=0; i<loop; i++ ){
		//等待Ping缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += bsize;
		//启动Pong缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_pong, bsize );
		//开始处理Ping缓存数据
		Roseek_RGB2YCrCb420( org_ping, y_ping, cr_ping, cb_ping, colsize, subrow );
		//启动YCrCb传输
		if( i>0 ){
			Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		}
		Roseek_DAT_Copy( g_hDAT5, y_ping, pDstY, bsize/3 );
		Roseek_DAT_Copy( g_hDAT6, cr_ping, pDstCr, bsize/12 );
		Roseek_DAT_Copy( g_hDAT7, cb_ping, pDstCb, bsize/12 );
		pDstY += bsize/3;
		pDstCr += bsize/12;
		pDstCb += bsize/12;
		//等待Pong缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += bsize;
		//启动Ping缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
		//开始处理Pong缓存数据
		Roseek_RGB2YCrCb420( org_pong, y_pong, cr_pong, cb_pong, colsize, subrow );
		//启动YCrCb传输
		Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		Roseek_DAT_Copy( g_hDAT5, y_pong, pDstY, bsize/3 );
		Roseek_DAT_Copy( g_hDAT6, cr_pong, pDstCr, bsize/12 );
		Roseek_DAT_Copy( g_hDAT7, cb_pong, pDstCb, bsize/12 );
		pDstY += bsize/3;
		pDstCr += bsize/12;
		pDstCb += bsize/12;
	}
	Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
}


/**
 * \function 	EE3_YUV2YCrCb420
 * \brief    	该函数用于将YUV422格式的图像原始数据转换成YUV420格式的数据，并将转换完毕的数据存放在用户指定的Y、Cr、Cb三个缓存中，用于JPEG编码
 * \			该模块的工作过程调用了智能相机系统的DAT模块，并占用了DAT模块的4、5、6、7通道。仔细阅读该模块可帮助用户了解DAT模块的工作原理	
 * \			
**/	
void EE3_YUV2YCrCb420( Uint8* pui8YUVInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 colsize, Uint32 rowsize)
{
	Uint32 i;
	Uint32 subrow = DAT_YUV_ROW_SIZE;
	Uint32 bsize = colsize*2*subrow;
	Uint32 loop = rowsize/subrow/2;
	Uint8* restrict pSrc = pui8YUVInBuf;
	Uint8* restrict pDstY = pui8YBuf;
	Uint8* restrict pDstCr = pui8CrBuf;
	Uint8* restrict pDstCb = pui8CbBuf;

	Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
	for( i=0; i<loop; i++ ){
		//等待Ping缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += bsize;
		//启动Pong缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_pong, bsize );
		//开始处理Ping缓存数据
		Roseek_YUV2YCrCb420( org_ping, y_ping, cr_ping, cb_ping, colsize, subrow );
		//启动YCrCb传输
		if( i>0 ){
			Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		}
		Roseek_DAT_Copy( g_hDAT5, y_ping, pDstY, bsize/2 );
		Roseek_DAT_Copy( g_hDAT6, cr_ping, pDstCr, bsize/8 );
		Roseek_DAT_Copy( g_hDAT7, cb_ping, pDstCb, bsize/8 );
		pDstY += bsize/2;
		pDstCr += bsize/8;
		pDstCb += bsize/8;
		//等待Pong缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += bsize;
		//启动Ping缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
		//开始处理Pong缓存数据
		Roseek_YUV2YCrCb420( org_pong, y_pong, cr_pong, cb_pong, colsize, subrow );
		//启动YCrCb传输
		Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		Roseek_DAT_Copy( g_hDAT5, y_pong, pDstY, bsize/2 );
		Roseek_DAT_Copy( g_hDAT6, cr_pong, pDstCr, bsize/8 );
		Roseek_DAT_Copy( g_hDAT7, cb_pong, pDstCb, bsize/8 );
		pDstY += bsize/2;
		pDstCr += bsize/8;
		pDstCb += bsize/8;
	}
	Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
}


/**
 * \function 	EE3_Bayer2YCrCb420
 * \brief    	该函数用于将Bayer格式的图像原始数据转换成YUV420格式的数据，并将转换完毕的数据存放在用户指定的Y、Cr、Cb三个缓存中，用于JPEG编码
 * \			该模块的工作过程调用了智能相机系统的DAT模块，并占用了DAT模块的4、5、6、7通道。仔细阅读该模块可帮助用户了解DAT模块的工作原理	
 * \			
**/	
void EE3_Bayer2YCrCb420( Uint8* pui8YUVInBuf, Uint8* pui8YBuf, Uint8* pui8CrBuf, Uint8* pui8CbBuf, Uint32 colsize, Uint32 rowsize)
{
	Uint32 i;
	Uint32 subrow = DAT_BAYER_ROW_SIZE;
	Uint32 bsize = colsize*(DAT_BAYER_ROW_SIZE+2);
	Uint32 loop = rowsize/subrow/2;
	Uint8* restrict pSrc = pui8YUVInBuf;
	Uint8* restrict pDstY = pui8YBuf;
	Uint8* restrict pDstCr = pui8CrBuf;
	Uint8* restrict pDstCb = pui8CbBuf;

	Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
	for( i=0; i<loop; i++ ){
		//等待Ping缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += colsize*subrow;
		//启动Pong缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_pong, bsize );
		//开始处理Ping缓存数据
		Roseek_Bayer2YCrCb420( org_ping, y_ping, cr_ping, cb_ping, 0, colsize, subrow+2 );
		//启动YCrCb传输
		if( i>0 ){
			Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
			Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		}
		Roseek_DAT_Copy( g_hDAT5, y_ping, pDstY, colsize*subrow );
		Roseek_DAT_Copy( g_hDAT6, cr_ping, pDstCr, colsize*subrow/4 );
		Roseek_DAT_Copy( g_hDAT7, cb_ping, pDstCb, colsize*subrow/4 );
		pDstY += colsize*subrow;
		pDstCr += colsize*subrow/4;
		pDstCb += colsize*subrow/4;
		//等待Pong缓存传输完毕
		Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
		pSrc += colsize*subrow;
		//启动Ping缓存传输
		Roseek_DAT_Copy( g_hDAT4, pSrc, org_ping, bsize );
		//开始处理Pong缓存数据
		Roseek_Bayer2YCrCb420( org_pong, y_pong, cr_pong, cb_pong, 0, colsize, subrow+2 );
		//
		Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
		Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
		//启动YCrCb传输
		Roseek_DAT_Copy( g_hDAT5, y_pong, pDstY, colsize*subrow );
		Roseek_DAT_Copy( g_hDAT6, cr_pong, pDstCr, colsize*subrow/4 );
		Roseek_DAT_Copy( g_hDAT7, cb_pong, pDstCb, colsize*subrow/4 );
		pDstY += colsize*subrow;
		pDstCr += colsize*subrow/4;
		pDstCb += colsize*subrow/4;
	}
	//end
	Roseek_DAT_Wait( g_hDAT4, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT5, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT6, DAT_WAITALL );
	Roseek_DAT_Wait( g_hDAT7, DAT_WAITALL );
}   



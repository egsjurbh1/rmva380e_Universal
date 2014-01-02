/**
 * \file      	EE3_ENCDataAcq.c
 * \author    	Tanggula Yang
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	相机的H.264视频流数据接收模块。
**/

#include "EE3_common.h"
/**************************************************************************/
Uint32 ui32DType, ui32DLen, ui32DResolutionType;
static Uint8 m_pEncFrameBuf[500*1024];
extern RoseekJPGEncBufStruct	g_JpgEncBufStruct;
void EE3_ENCJpgAcq_Config();
static	Uint8	ui8JpgBufIndex=0;

#ifndef DATAPOOLITEM_DEFINE
#define DATAPOOLITEM_DEFINE
typedef struct tagDataPoolItem
{
	Uint8 *pBuf;
}DataPoolItem;
#endif
/**************************************************************************/
extern int	g_nMaxFrameSizeLiveStream;
extern int	g_nMaxBuffsNumLiveStream;
extern DataPoolItem *g_pBuffsLiveStream;
extern volatile int	g_nWritePosLSBuffs;
extern volatile int	g_nReadPosLSBuffs;
extern volatile int	g_nNumHasDataLSBuffs;
extern volatile Bool g_bIsLiveStreamClientConnect;
/**************************************************************************/
extern int	g_nMaxFrameSizeLiveStream_Another;
extern int	g_nMaxBuffsNumLiveStream_Another;
extern DataPoolItem *g_pBuffsLiveStream_Another;
extern volatile int	g_nWritePosLSBuffs_Another;
extern volatile int	g_nReadPosLSBuffs_Another;
extern volatile int	g_nNumHasDataLSBuffs_Another;
extern volatile Bool g_bIsLiveStreamClientConnect_Another;
/**************************************************************************/
extern volatile Bool g_bIsLiveStreamClientConnect_rtsp;
extern volatile Bool g_bIsLiveStreamClientConnect_rtsp_ch1;

extern void Roseek_Stop_Enc();
extern void Roseek_Start_Enc();
extern volatile int isEncStop;


/**
 * \function	hwiEncDataAcqFunc
 * \brief		H.264数据传输事件硬件中断响应函数(硬件中断)
 * \			当H.264编码模块将视频流编码后开始传输到DSP端，或者DSP端接收完毕一帧压缩好的H.264视频数据，该中断响应函数都会自动被调用
 * \note		不允许用户更改本函数名	
 */
void hwiEncDataAcqFunc()
{
	if( g_bIsLiveStreamClientConnect || g_bIsLiveStreamClientConnect_Another 
	|| g_bIsLiveStreamClientConnect_rtsp || g_bIsLiveStreamClientConnect_rtsp_ch1 )
	{
		if( Roseek_IsENCCodeStream_Rdy() )
		{
			Roseek_ENCDataAcq_Config(&ui32DType, &ui32DLen, m_pEncFrameBuf+4);
			return;
		}
		if( Roseek_Encd_TransOver() )
		{
			//请在此处自行维护CACHE，否则数据上传会有误
			if (ui32DType == ENC_JPG)
			{
				BCACHE_inv( g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex], ui32DLen+4 , TRUE );
				*(Uint32*)g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex] = ui32DLen;
				g_JpgEncBufStruct.bJpgImgBuf_Lock[ui8JpgBufIndex] = TRUE;
			}
			else if (ui32DType == ENC_H264)
			{
				BCACHE_inv( m_pEncFrameBuf, ui32DLen+4 , TRUE );
				ui32DResolutionType = *(m_pEncFrameBuf + ui32DLen);

				ui32DLen -= 32;
				//原始分辨率
				
				if (ui32DResolutionType == 0)
				{
					if (g_bIsLiveStreamClientConnect || g_bIsLiveStreamClientConnect_rtsp)
					{
						if (g_nNumHasDataLSBuffs >= g_nMaxBuffsNumLiveStream)
						{
							LOG_printf( &trace, "buff full, throw!!!");
						}
						else if (g_nNumHasDataLSBuffs >= (g_nMaxBuffsNumLiveStream-1) && (m_pEncFrameBuf[8]&0x1F) != 7)
						{
							LOG_printf( &trace, "this frame throw!!!");
						}
						else
						{
							*(Uint32*)(g_pBuffsLiveStream[g_nWritePosLSBuffs].pBuf) = ui32DLen;
							memcpy(g_pBuffsLiveStream[g_nWritePosLSBuffs].pBuf + 4, m_pEncFrameBuf + 4, ui32DLen);
							g_nWritePosLSBuffs = (++g_nWritePosLSBuffs)%g_nMaxBuffsNumLiveStream;
							g_nNumHasDataLSBuffs++;
							SEM_post( &sem_LiveStreamDataReady);
						}
					}
				}
				//变种分辨率
				else if (ui32DResolutionType == 4)
				{
					if (g_bIsLiveStreamClientConnect_Another || g_bIsLiveStreamClientConnect_rtsp_ch1)
					{
						if (g_nNumHasDataLSBuffs_Another >= g_nMaxBuffsNumLiveStream_Another)
						{
							LOG_printf( &trace, "buff full, throw!!!");
						}
						else if (g_nNumHasDataLSBuffs_Another >= (g_nMaxBuffsNumLiveStream_Another-1) && (m_pEncFrameBuf[8]&0x1F) != 7)
						{
							LOG_printf( &trace, "this frame throw!!!");
						}
						else
						{
							*(Uint32*)(g_pBuffsLiveStream_Another[g_nWritePosLSBuffs_Another].pBuf) = ui32DLen;
							memcpy(g_pBuffsLiveStream_Another[g_nWritePosLSBuffs_Another].pBuf + 4, m_pEncFrameBuf + 4, ui32DLen);
							g_nWritePosLSBuffs_Another = (++g_nWritePosLSBuffs_Another)%g_nMaxBuffsNumLiveStream_Another;
							g_nNumHasDataLSBuffs_Another++;
							SEM_post( &sem_LiveStreamDataReady_Another);
						}
					}
				}
				else
				{
					LOG_printf( &trace, "unkonwn type %d\n", ui32DResolutionType);
				}
				
			}
		}
	}
}

void EE3_ENCJpgAcq_Config()
{
	Bool bNobuf = TRUE;
	Uint8	i, ui8tmp = 0;

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
				bNobuf = FALSE;
				break;
			}
		}
		else{//i<=5
			if( !g_JpgEncBufStruct.bJpgImgBuf_Lock[i] ){
				ui8JpgBufIndex = i;
				bNobuf = FALSE;
				break;
			}
		}
	}
	if( bNobuf ){
		return;
	}
	Roseek_ENCDataAcq_Config( &ui32DType, &ui32DLen, g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex]+4 );
}


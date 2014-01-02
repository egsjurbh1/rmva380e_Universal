/**
 * \file      	H264MediaSourceLiveDsp.c
 * \author    	Tanggula Yang
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	H.264ÏµÍ³Ä£¿é
 **/


#include "H264MediaSourceLiveDsp.h"
#include "MediaSource.h"

#ifndef DATAPOOLITEM_DEFINE
#define DATAPOOLITEM_DEFINE
typedef struct tagDataPoolItem
{
	Uint8 *pBuf;
}DataPoolItem;
#endif
/**************************/
extern int	g_nMaxFrameSizeLiveStream;
extern int	g_nMaxBuffsNumLiveStream;
extern DataPoolItem *g_pBuffsLiveStream;
extern volatile int	g_nWritePosLSBuffs;
extern volatile int	g_nReadPosLSBuffs;
extern volatile int	g_nNumHasDataLSBuffs;
extern volatile Bool g_bIsLiveStreamClientConnect;
/*************************/
extern int	g_nMaxFrameSizeLiveStream_Another;
extern int	g_nMaxBuffsNumLiveStream_Another;
extern DataPoolItem *g_pBuffsLiveStream_Another;
extern volatile int	g_nWritePosLSBuffs_Another;
extern volatile int	g_nReadPosLSBuffs_Another;
extern volatile int	g_nNumHasDataLSBuffs_Another;
extern volatile Bool g_bIsLiveStreamClientConnect_Another;
/*************************/

extern void Roseek_Stop_Enc();
extern void Roseek_Start_Enc();
extern volatile int isEncStop;
extern volatile int	g_IsUsedByTCPSend;
extern volatile int	g_IsUsedByTCPSend_Another;

volatile Bool g_bIsLiveStreamClientConnect_rtsp = FALSE;
volatile Bool g_bIsLiveStreamClientConnect_rtsp_ch1 = FALSE;

extern far SEM_Obj sem_LiveStreamDataReady;
extern far SEM_Obj sem_LiveStreamDataReady_Another;

int rtsp_mediaSource_h264LiveDsp_open(MediaSource_t *pSource)
{
	int ret = -1;

	do 
	{
		pSource->frequency = 90000;
		pSource->fps = 25;

		if (pSource->channel == 0)
		{
			g_bIsLiveStreamClientConnect_rtsp = TRUE;
		}
		else if (pSource->channel == 1)
		{
			g_bIsLiveStreamClientConnect_rtsp_ch1 = TRUE;
		}
		else
		{
			break;
		}

		ret = 0;
	} while(0);

	return ret;
}

int rtsp_mediaSource_h264LiveDsp_close(MediaSource_t *pSource)
{
	if (pSource->channel == 0)
	{
		g_bIsLiveStreamClientConnect_rtsp = FALSE;
	}
	else if (pSource->channel == 1)
	{
		g_bIsLiveStreamClientConnect_rtsp_ch1 = FALSE;
	}

	return 0;
}


int rtsp_mediaSource_h264LiveDsp_getOneFrame(MediaSource_t *pSource, uint8_t *pBuf, int bufLen, uint32_t *pDuration)
{
	int ret = 0;
	int frameSize;

	do 
	{
		if (pSource->channel == 0)
		{
			if (g_IsUsedByTCPSend)
			{
				TSK_sleep(50);
				ret = 0;
				break;
			}

			if(isEncStop && g_nNumHasDataLSBuffs <= 1)
			{
				Roseek_Start_Enc();
				isEncStop = 0;
			}

			if (!SEM_pend( &sem_LiveStreamDataReady, 10) || g_nNumHasDataLSBuffs <= 0)
			{
				ret = 0;
				break;
			}

			if (g_nNumHasDataLSBuffs >= (g_nMaxBuffsNumLiveStream-1) && (g_pBuffsLiveStream[g_nReadPosLSBuffs].pBuf[8]&0x1F) != 7)
			{
				ret = 0;
			}
			else
			{
				frameSize = *(uint32_t*)(g_pBuffsLiveStream[g_nReadPosLSBuffs].pBuf);
				if (frameSize > bufLen)
				{
					ret = 0;
				}
				else
				{
					memcpy(pBuf, g_pBuffsLiveStream[g_nReadPosLSBuffs].pBuf + 4, frameSize);
					ret = frameSize;
				}	
			}			

			g_nReadPosLSBuffs = (++g_nReadPosLSBuffs)%g_nMaxBuffsNumLiveStream;
			g_nNumHasDataLSBuffs--;
		}
		else if (pSource->channel == 1)
		{
			if (g_IsUsedByTCPSend_Another)
			{
				TSK_sleep(50);
				ret = 0;
				break;
			}

			if(isEncStop && g_nNumHasDataLSBuffs_Another <= 1)
			{
				Roseek_Start_Enc();
				isEncStop = 0;
			}

			if (!SEM_pend( &sem_LiveStreamDataReady_Another, 10) || g_nNumHasDataLSBuffs_Another <= 0)
			{
				ret = 0;
				break;
			}

			if (g_nNumHasDataLSBuffs_Another >= (g_nMaxBuffsNumLiveStream_Another-1) && (g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf[8]&0x1F) != 7)
			{
				ret = 0;
			}
			else
			{
				frameSize = *(uint32_t*)(g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf);
				if (frameSize > bufLen)
				{
					ret = 0;
				}
				else
				{
					memcpy(pBuf, g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf + 4, frameSize);
					ret = frameSize;
				}	
			}			

			g_nReadPosLSBuffs_Another = (++g_nReadPosLSBuffs_Another)%g_nMaxBuffsNumLiveStream_Another;
			g_nNumHasDataLSBuffs_Another--;
		}
		else
		{
			TSK_sleep(50);
			ret = 0;
			break;
		}

		*pDuration = pSource->frequency/pSource->fps;
		
	} while(0);

	return ret;
}


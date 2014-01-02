/**
 * \file      	rtspMain.c
 * \author    	Tanggula Yang
 * \version   	1.01
 * \date      	2012.07.13
 * \brief     	H.264系统模块
 **/

#include "EE3_common.h"
#include "RtspCommon.h"
#include "RtspServer.h"
#include "MediaSession.h"
#include "MediaSubSession.h"

extern unsigned int	g_IPAddrCurUse;

void rtspMainThread()
{
	int ret = 0;
	RtspServer_t *pRtspServer = NULL;
	MediaSession_t *pH264Media = NULL;
	MediaSubSession_t *pH264MediaSubSession = NULL;
	MediaSession_t *pH264Media_ch1 = NULL;
	MediaSubSession_t *pH264MediaSubSession_ch1 = NULL;
	uint8_t *pIP;
	uint32_t ip;
	char szIP[40];

	fdOpenSession( TaskSelf() );

	do 
	{
		ip = g_IPAddrCurUse;
		pIP = (uint8_t*)&ip;
		sprintf(szIP, "%u.%u.%u.%u", pIP[0], pIP[1], pIP[2], pIP[3]);

		//第4个参数，rtsp中线程的优先级最好是比所有网络线程和高密度运算线程的优先级高一级
		ret = rtsp_buildServer(&pRtspServer, szIP, 8557, 6);
		if (ret != 0)
		{
			break;
		}

		//*********ch0**********//
		ret = rtsp_buildMediaSubSession(&pH264MediaSubSession, SOURCE_TYPE_H264_LIVE_DSP, 0);
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_buildMediaSession(&pH264Media, "h264");
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_addMediaSubSession(pH264Media, pH264MediaSubSession);
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_addMediaSession(pRtspServer, pH264Media);
		if (ret != 0)
		{
			break;
		}
		//**********ch1*********//
		ret = rtsp_buildMediaSubSession(&pH264MediaSubSession_ch1, SOURCE_TYPE_H264_LIVE_DSP, 1);
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_buildMediaSession(&pH264Media_ch1, "h264_ch1");
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_addMediaSubSession(pH264Media_ch1, pH264MediaSubSession_ch1);
		if (ret != 0)
		{
			break;
		}

		ret = rtsp_addMediaSession(pRtspServer, pH264Media_ch1);
		if (ret != 0)
		{
			break;
		}
		//*********************//

		ret = rtsp_run(pRtspServer);
		if (ret != 0)
		{
			break;
		}

		while (1)
		{
			TSK_sleep(1000);
		}

//		rtsp_stop(pRtspServer);

	} while(0);

	rtsp_clearServer(pRtspServer);
	rtsp_clearMediaSession(pH264Media);
	rtsp_clearMediaSubSession(pH264MediaSubSession);
	rtsp_clearMediaSession(pH264Media_ch1);
	rtsp_clearMediaSubSession(pH264MediaSubSession_ch1);
}

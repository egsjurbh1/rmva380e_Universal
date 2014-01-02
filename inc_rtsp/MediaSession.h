#ifndef _MEDIA_SESSION_H_
#define _MEDIA_SESSION_H_

#include "RtspCommon.h"

#define MAX_SUB_SESSION_NUM			5
#define MAX_MEDIA_NAME_SIZE			50
#define MAX_SDP_DESCRIPTION_SIZE	2*1024

typedef struct tagMediaSubSession MediaSubSession_t;

typedef struct tagMediaSession 
{
	MediaSubSession_t	*SubSession[MAX_SUB_SESSION_NUM];
	char				szName[MAX_MEDIA_NAME_SIZE];
	int					useCount;
}MediaSession_t;

int rtsp_buildMediaSession(MediaSession_t **ppMediaSession, const char *pName);
int rtsp_clearMediaSession(MediaSession_t *pMediaSession);
int rtsp_addMediaSubSession(MediaSession_t *pMediaSession, MediaSubSession_t *pSubSession);
MediaSubSession_t * rtsp_loopupMediaSubSession(MediaSession_t *pMediaSession, const char *pName);
int rtsp_getSDPDescription(MediaSession_t *pMediaSession, char *pSDPBuf, int bufLen);

#endif



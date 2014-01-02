#ifndef _RTSP_SERVER_H_
#define _RTSP_SERVER_H_

#include "RtspCommon.h"
#include "thread.h"
#include "sock.h"

#define MAX_MEDIA_SESSION_NUM		10
#define MAX_CLIENT_SESSION_NUM		10

typedef struct tagMediaSession MediaSession_t;
typedef struct tagRtspSession RtspSession_t;

typedef struct tagRtspServer 
{
	SOCKET_TYPE			fd;
	uint16_t			port;
	char				szIP[MAX_NAME_SIZE];
	MediaSession_t		*mediaSessions[MAX_MEDIA_SESSION_NUM];
	RtspSession_t		*clientSessions[MAX_CLIENT_SESSION_NUM];
	int					needExit;
	TGL_thread_t		*pThreadServe;
	uint32_t			sessionIDCounter;
}RtspServer_t;

int rtsp_buildServer(RtspServer_t **ppRtspServer, const char *pIP, uint16_t listenPort, int rtspThreadPriority);
int rtsp_clearServer(RtspServer_t *pRtspServer);
int rtsp_run(RtspServer_t *pRtspServer);
int rtsp_stop(RtspServer_t *pRtspServer);
int rtsp_addMediaSession(RtspServer_t *pRtspServer, MediaSession_t *pMediaSession);
int rtsp_getClientSessionNum(RtspServer_t *pRtspServer);
MediaSession_t* rtsp_loopupMediaSession(RtspServer_t *pRtspServer, const char *pName);
int rtsp_generateMediaUrl(RtspServer_t *pRtspServer, MediaSession_t *pMediaSession, char *pUrlBuf, int bufLen);


#endif


#ifndef _MEDIA_SUB_SESSION_H_
#define _MEDIA_SUB_SESSION_H_

#include "RtspCommon.h"

typedef struct tagMediaInstance MediaInstance_t;

#ifndef _TAG_MEDIASUBSESSION
#define _TAG_MEDIASUBSESSION
typedef struct tagMediaSubSession 
{
	MediaInstance_t	*pInstanceSaved;
	int				sourceType;
	int				channel;
	char			szTrackName[MAX_NAME_SIZE];
}MediaSubSession_t;
#endif

int rtsp_buildMediaSubSession(MediaSubSession_t **ppSubSession, int sourceType, int channel);
int rtsp_clearMediaSubSession(MediaSubSession_t *pSubSession);
MediaInstance_t * rtsp_mediaSubSession_createMediaInstance(MediaSubSession_t *pSubSession, int reuse);
int rtsp_mediaSubSession_getSdplines(MediaSubSession_t *pSubSession, char *pSdplinesBuf, int bufLen);

#endif


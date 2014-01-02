#ifndef _MEDIA_SOURCE_H_
#define _MEDIA_SOURCE_H_

#include "RtspCommon.h"
#include "sock.h"

typedef struct tagMediaSource 
{
	char	szName[MAX_NAME_SIZE];
	int		sourceType;
	int		channel;
	SOCKET_TYPE	fd;
	int		frequency;
	int		fps;
}MediaSource_t;

int rtsp_buildMediaSource(MediaSource_t **ppMediaSource, int sourceType, int channel);
int rtsp_clearMediaSource(MediaSource_t *pMediaSource);
int rtsp_mediaSource_open(MediaSource_t *pMediaSource);
int rtsp_mediaSource_close(MediaSource_t *pMediaSource);
int rtsp_mediaSource_getMediaData(MediaSource_t *pMediaSource, uint8_t *pDataBuf, int bufLen, uint32_t *pDuration);

#endif



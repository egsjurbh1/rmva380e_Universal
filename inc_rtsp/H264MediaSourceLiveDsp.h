#ifndef _H264_MEDIA_SOURCE_LIVEDSP_H_
#define _H264_MEDIA_SOURCE_LIVEDSP_H_

#include "RtspCommon.h"

typedef struct tagMediaSource MediaSource_t;

int rtsp_mediaSource_h264LiveDsp_getOneFrame(MediaSource_t *pSource, uint8_t *pBuf, int bufLen, uint32_t *pDuration);
int rtsp_mediaSource_h264LiveDsp_open(MediaSource_t *pSource);
int rtsp_mediaSource_h264LiveDsp_close(MediaSource_t *pSource);

#endif



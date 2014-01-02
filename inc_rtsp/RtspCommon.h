#ifndef _RTSP_COMMON_H_
#define _RTSP_COMMON_H_

#define PLATFORM_DSP

#ifdef WIN32
#include <stdlib.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#else
#ifdef PLATFORM_DSP
#include <std.h>
#include <tsk.h>
#include <c64.h>
#include <stdlib.h>
#include <csl_types.h>
#include <soc.h>
#include <bcache.h>
#include <cslr.h>
#include <stdio.h>
#include <log.h>
#include <tistdtypes.h>
#include <cslr_cache.h>
#include <c6x.h>
#include <cslr_i2c.h>
#include <cslr_pllc1.h>
#include <cslr_tmr.h>
#include <cslr_psc.h>
#include <cslr_sys.h>
#include <string.h>
#include <math.h>
#include <netmain.h>
#include "EE3_common.h"
#endif
#endif

typedef signed char			int8_t;
typedef signed short		int16_t;
typedef signed int			int32_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;

#ifdef WIN32
typedef signed __int64		int64_t;
typedef unsigned __int64	uint64_t;
typedef void				*HANDLE;
#else
#ifdef PLATFORM_DSP
//typedef signed __int64		int64_t;
//typedef unsigned __int64	uint64_t;
#endif
#endif


//extern void Roseek_RTC_ReadTime(uint8_t *);


#define MAX_NAME_SIZE		100

#ifndef NULL
#define NULL	0
#endif

enum RTSP_SOURCE_TYPE
{
	SOURCE_TYPE_H264_LIVE_DSP,
	SOURCE_TYPE_H264_FILE,
	SOURCE_TYPE_MPEG4
};

enum RTSP_STREAMING_MODE
{
	STREAMING_MODE_RTP_UDP,
	STREAMING_MODE_RTP_TCP,
	STREAMING_MODE_RAW_UDP
};

enum RTSP_STATE
{
	RTSP_INIT,
	RTSP_READY,
	RTSP_PLAYING,
	RTSP_RECORD,
	RTSP_NONE
};

#define RTP_VERSION			2

#endif


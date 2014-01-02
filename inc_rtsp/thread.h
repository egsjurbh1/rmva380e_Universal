#ifndef _THREAD_H_
#define _THREAD_H_

#include "RtspCommon.h"

#define DEFAULT_THREAD_PRIORITY		6
#define DEFAULT_TRHEAD_STACKSIZE	0x1400

typedef int (*THREAD_FUNC)(void *pParam);

typedef struct tagTGLThread 
{
	HANDLE			hThread;
	THREAD_FUNC		func;
	void			*pParam;
	char			szName[MAX_NAME_SIZE];
}TGL_thread_t;

int thread_create(TGL_thread_t **ppTGLThread, THREAD_FUNC func, void *pParam, int priority);
int	thread_destroy(TGL_thread_t *pTGLThread, int timeout);

#endif


/**
 * \file      	EE3_SendLiveStreamData_Another.c
 * \author    	Tanggula Yang
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	H.264子码流的发送线程
**/
#include "EE3_common.h"


/**************************************************************************/
//外部变量
extern int extHeap;
/**************************************************************************/
#define MAX_FRAME_SIZE_LIVESTREAM		500*1024
#define	MAX_BUFFS_NUM_LIVESTREAM		10
int		g_nMaxFrameSizeLiveStream_Another = MAX_FRAME_SIZE_LIVESTREAM;
int		g_nMaxBuffsNumLiveStream_Another = MAX_BUFFS_NUM_LIVESTREAM;
volatile int	g_nWritePosLSBuffs_Another = 0;
volatile int	g_nReadPosLSBuffs_Another = 0;
volatile int	g_nNumHasDataLSBuffs_Another = 0;
volatile Bool	g_bIsLiveStreamClientConnect_Another = FALSE;

extern void Roseek_Stop_Enc();
extern void Roseek_Start_Enc();
extern volatile int isEncStop;
volatile int g_IsUsedByTCPSend_Another = FALSE;

#ifndef DATAPOOLITEM_DEFINE
#define DATAPOOLITEM_DEFINE
typedef struct tagDataPoolItem
{
	Uint8 *pBuf;
}DataPoolItem;
#endif
DataPoolItem *g_pBuffsLiveStream_Another = 0;
static int nSendLen = 0, nDataLen = 0;
/**************************************************************************/
static int SafeSend(SOCKET sock, Uint8* pBuf, int nLen);


static int sock_setTimeout(SOCKET sock, int op, int timeout)
{
         struct timeval time;
         time.tv_sec = timeout/1000;
         time.tv_usec = (timeout%1000)*1000;
 
         return setsockopt(sock, SOL_SOCKET, op, &time, sizeof(time));
}
 
/**
 * \function 	SendLiveStreamDataPort
 * \brief    	使用TCP/IP协议实现H.264子码流的网络传输
 * \			默认作为服务器端，Port = 61001		
 * \note		除了TCP/IP的传输方式外，智能相机系统还支持H.264视频流的RTSP方式传输，其传输过程已被封装在系统内部，默认TCP/IP传输的优先级高于RTSP传输的优先级
**/	
void SendLiveStreamDataPort_Another()
{
	SOCKET	sockThis, sockAccept;
	struct	sockaddr_in addrThis, addrAccept;
	int		nAddrLen;
	int		nVideoWidth = 1600, nVideoHeight = 1216;
	int 	i;
	int		isValidLink = 1;

	fdOpenSession( TaskSelf() );

	do
	{
		g_pBuffsLiveStream_Another = MEM_alloc(extHeap, g_nMaxBuffsNumLiveStream_Another*sizeof(DataPoolItem), 256);
	}while(g_pBuffsLiveStream_Another == 0);
	
	memset(g_pBuffsLiveStream_Another, 0, g_nMaxBuffsNumLiveStream_Another*sizeof(DataPoolItem));
	for (i = 0; i < g_nMaxBuffsNumLiveStream_Another; i++)
	{
		do
		{
			g_pBuffsLiveStream_Another[i].pBuf = MEM_alloc(extHeap, g_nMaxFrameSizeLiveStream_Another, 256);
		}while(g_pBuffsLiveStream_Another[i].pBuf == 0);	
	}

	sockThis = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( sockThis == INVALID_SOCKET )
	{
		Roseek_Reset();//如果创建侦听对象失败，重启相机
	}

	bzero( &addrAccept, sizeof(addrAccept) );
	nAddrLen = sizeof(addrAccept);
	bzero( &addrThis, sizeof(addrThis) );
	addrThis.sin_len	= sizeof( addrThis );
	addrThis.sin_family	= AF_INET;
	addrThis.sin_addr.s_addr = INADDR_ANY;
	addrThis.sin_port	= htons( 61001 );
	//sockFconListen绑定
	if ( bind( sockThis, (PSA) &addrThis, sizeof(addrThis) ) < 0 )
	{
		Roseek_Reset();//如果绑定失败，重启相机
	}
	//sockFconListen开始监听，同一时刻仅支持一个连接
	if ( listen( sockThis, 1) < 0 ){
		Roseek_Reset();//如果侦听失败，重启相机
	}
	//迭代循环
	while( 1 )
	{
		sockAccept = INVALID_SOCKET;
		do
		{
			sockAccept = accept( sockThis, (PSA)&addrAccept, &nAddrLen );
			if( sockAccept == INVALID_SOCKET)
			{
				break;
			}
			
			sock_setTimeout(sockAccept, SO_RCVTIMEO, 3000);
     	sock_setTimeout(sockAccept, SO_SNDTIMEO, 3000);

			g_bIsLiveStreamClientConnect_Another = TRUE;
			g_IsUsedByTCPSend_Another = TRUE;
			if (SafeSend(sockAccept, (Uint8*)&nVideoWidth, 4) != 4)
			{
				break;
			}
			if (SafeSend(sockAccept, (Uint8*)&nVideoHeight, 4) != 4)
			{
				break;
			}

			while( 1 )
			{
				if(isEncStop && g_nNumHasDataLSBuffs_Another <= 1)
				{
					Roseek_Start_Enc();
					isEncStop = 0;
					LOG_printf( &trace, "Roseek_Start_Enc!!!");
				}

				//等待信号灯同步
				isValidLink = 1;
				while (!SEM_pend( &sem_LiveStreamDataReady_Another, 5000))
				{
					int nRet;
					struct timeval tvTimeOut;
					fd_set fsVal;
					FD_ZERO(&fsVal);
					FD_SET(sockAccept, &fsVal);
					tvTimeOut.tv_sec = 0;
					tvTimeOut.tv_usec = 100;
					nRet = fdSelect(0, 0, &fsVal, 0, &tvTimeOut);
					if (0 == nRet || -1 == nRet)
					{
						isValidLink = 0;
						break;
					}
				}
				//SEM_pend( &sem_LiveStreamDataReady_Another, SYS_FOREVER );

				if (!isValidLink)
				{
					break;
				}
				
				//判断上传哪个缓冲区内容
				
				if (g_nNumHasDataLSBuffs_Another <= 0)
				{
					continue;
				}

				if (!isEncStop && (g_nNumHasDataLSBuffs_Another >= (g_nMaxBuffsNumLiveStream_Another-3)))
				{
					Roseek_Stop_Enc();
					isEncStop = 1;
					LOG_printf( &trace, "Roseek_Stop_Enc!!!");
				}

				if (g_nNumHasDataLSBuffs_Another >= (g_nMaxBuffsNumLiveStream_Another-1) && (g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf[8]&0x1F) != 7)
				{
					LOG_printf( &trace, "this frame not send!!!");
				}
				else
				{
					nDataLen = *(Uint32*)(g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf) + 4;
					if((nSendLen = SafeSend( sockAccept, g_pBuffsLiveStream_Another[g_nReadPosLSBuffs_Another].pBuf, nDataLen)) != nDataLen)
					{
							LOG_printf( &trace, "send error!!!");
							break;
					}
				}			
				
				g_nReadPosLSBuffs_Another = (++g_nReadPosLSBuffs_Another)%g_nMaxBuffsNumLiveStream_Another;
				g_nNumHasDataLSBuffs_Another--;
				
			}
		}while(0);
		
		if (sockAccept != INVALID_SOCKET)
		{
			fdClose(sockAccept);
			sockAccept = INVALID_SOCKET;
		}
		
		g_IsUsedByTCPSend_Another = FALSE;
		g_bIsLiveStreamClientConnect_Another = FALSE;
		SEM_reset( &sem_LiveStreamDataReady_Another, 0 );
		g_nNumHasDataLSBuffs_Another = 0;
		g_nReadPosLSBuffs_Another = 0;
		g_nWritePosLSBuffs_Another = 0;

		if(isEncStop)
		{
			Roseek_Start_Enc();
			isEncStop = 0;
			LOG_printf( &trace, "Roseek_Start_Enc!!!");
		}
	}//迭代循环
}

int SafeSend(SOCKET sock, Uint8* pBuf, int nLen)
{
	int nRet = 0;
	int nSended = 0;
	while (nSended < nLen)
	{
		nRet = send(sock, pBuf + nSended, nLen - nSended, 0);
		if (nRet <= 0)
		{
			break;
		}
		nSended += nRet;
	}
	
	return nSended;
}

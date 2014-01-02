/**
 * \file      	EE3_SendSDFSData.c
 * \author    	Tanggula Yang
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	SD卡内存储的图片数据访问、发送模块
**/

#include "EE3_common.h"
#include "netmain.h"
#include "SDFS_filesys.h"

#define MAX_FILE_NAME_LEN	19
#define MAX_FILE_SIZE		(2 * 1024 * 1024)
volatile Uint8 	g_ui8SDFSCMD = 0;
char	g_szParam1[MAX_FILE_NAME_LEN];
char	g_szParam2[MAX_FILE_NAME_LEN];	

extern void Roseek_Reset();
extern int extHeap;	//堆声明，在cdb文件中定义
extern SEM_Handle sem_SDFSInitReady;

static void Uint32ToBuf(Uint8 *pBuf, Uint32 ui32Value)
{
	*((Uint32*)pBuf) = ui32Value;
}

/**
 * \function 	SendSDFSDataPort
 * \brief    	使用TCP/IP协议实现SD卡内存储数据的网络访问、传输
 * \			默认作为服务器端，Port = 46000		
 * \note		使用此线层读取数据的前提是用户必须按照SD FS的格式进行SD卡内的数据填写
**/	
void SendSDFSDataPort()
{
	SOCKET	sockSDFSListen;
	SOCKET	sockSDFSSvr;
	struct	sockaddr_in	addr;
	int		iSize;
	Uint8	ui8Ret;
	Uint8	*pui8DataBuf = NULL;
	Uint32	ui32DataBufSize = 0;
	Uint8 	aui8Temp[4];
	
	//TSK_sleep(10000);
	//初始SD卡文件系统模块
	RS_FS_Initialize(50*1024, 512, 2048, 3);
	/*等待文件系统初始完毕*/
	SEM_pend(sem_SDFSInitReady, SYS_FOREVER);
	SEM_post(sem_SDFSInitReady);
	
	ui32DataBufSize = MAX_FILE_SIZE;
	pui8DataBuf = MEM_alloc(extHeap, ui32DataBufSize, 256);
	
	fdOpenSession(TaskSelf());
	
	sockSDFSListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ( sockSDFSListen == INVALID_SOCKET)
	{
		Roseek_Reset();
	}
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_len = sizeof(addr);
	addr.sin_port = htons(46000);
	
	if (bind(sockSDFSListen, (PSA)&addr, sizeof(addr)) < 0)
	{
		Roseek_Reset();
	}
	
	if (listen(sockSDFSListen, 1) < 0)
	{
		Roseek_Reset();
	}
	
	while (1)
	{
		iSize = sizeof(addr);
		sockSDFSSvr = accept(sockSDFSListen, (PSA)&addr, &iSize);
		if (sockSDFSSvr == INVALID_SOCKET)
		{
			fdClose(sockSDFSSvr);
			continue;
		}
		
		if (0 == g_ui8SDFSCMD)
		{
			fdClose(sockSDFSSvr);
			continue;
		}
		
		switch (g_ui8SDFSCMD)
		{
			/*获取两个日期内的所有图片文件名*/
			case 1:
			{
				Uint32 ui32FileNum = 0;
				ui8Ret = RS_FS_Find(&ui32FileNum, g_szParam1, g_szParam2);
				if (0 == ui8Ret && ui32FileNum > 0)
				{
					Uint32 i = 0;
					char szFileName[MAX_FILE_NAME_LEN];
					Uint32ToBuf(aui8Temp, ui32FileNum);
					send(sockSDFSSvr, aui8Temp, 4, 0);
					Uint32ToBuf(aui8Temp, MAX_FILE_NAME_LEN);					
					for (; i < ui32FileNum; i++)
					{
						send(sockSDFSSvr, aui8Temp, 4, 0);
						RS_FS_GetNext(szFileName, MAX_FILE_NAME_LEN, i);
						send(sockSDFSSvr, szFileName, MAX_FILE_NAME_LEN, 0); 
						if (i%100 == 0)
						{
							TSK_sleep(100);
						}
					}
				}
			}
			break;
			/*获取两个日期内的所有图片文件数据*/
			case 2:
			{
				Uint32 ui32FileNum = 0;
				ui8Ret = RS_FS_Find(&ui32FileNum, g_szParam1, g_szParam2);
				if (0 == ui8Ret && ui32FileNum > 0)
				{
					Uint32 i = 0;
					Uint32 ui32FileSize;
					char szFileName[MAX_FILE_NAME_LEN];
					Uint32ToBuf(aui8Temp, ui32FileNum);
					send(sockSDFSSvr, aui8Temp, 4, 0);					
					for (; i < ui32FileNum; i++)
					{
						RS_FS_GetNext(szFileName, MAX_FILE_NAME_LEN, i);
						RS_FS_GetFileSize(&ui32FileSize, szFileName);
						if (ui32FileSize > ui32DataBufSize)
						{
							ui32FileSize = ui32DataBufSize;
						}
						RS_FS_Read(pui8DataBuf, ui32DataBufSize, szFileName);
						Uint32ToBuf(aui8Temp, ui32FileSize + MAX_FILE_NAME_LEN);
						send(sockSDFSSvr, aui8Temp, 4, 0);
						send(sockSDFSSvr, szFileName, MAX_FILE_NAME_LEN, 0); 
						send(sockSDFSSvr, pui8DataBuf, ui32FileSize, 0);

						if (i%100 == 0)
						{
							TSK_sleep(100);
						}
					}
				}
			}
			break;
			/*删除两个日期内的所有图片文件*/
			case 3:
			{
				char szReturn[10];
				ui8Ret = RS_FS_DelByDate(g_szParam1, g_szParam2);
				if (0 == ui8Ret)
				{
					strcpy(szReturn, "success");
				}
				else
				{
					strcpy(szReturn, "fail");
				}
				Uint32ToBuf(aui8Temp, 1);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, 10);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				send(sockSDFSSvr, szReturn, 10, 0);
			}
			break;
			/*获取某个文件名的图片数据*/
			case 4:
			{
				Uint32 ui32FileSize;
				RS_FS_GetFileSize(&ui32FileSize, g_szParam1);
				if (ui32FileSize > ui32DataBufSize)
				{
					ui32FileSize = ui32DataBufSize;
				}
				RS_FS_Read(pui8DataBuf, ui32DataBufSize, g_szParam1); 
				Uint32ToBuf(aui8Temp, 1);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, ui32FileSize + MAX_FILE_NAME_LEN);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				send(sockSDFSSvr, g_szParam1, MAX_FILE_NAME_LEN, 0);
				send(sockSDFSSvr, pui8DataBuf, ui32FileSize, 0);				
			}
			break;			
			/*删除某个图片文件*/
			case 5:
			{
				char szReturn[10];
				ui8Ret = RS_FS_Del(g_szParam1);
				if (0 == ui8Ret)
				{
					strcpy(szReturn, "success");
				}
				else
				{
					strcpy(szReturn, "fail");
				}
				Uint32ToBuf(aui8Temp, 1);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, 10);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				send(sockSDFSSvr, szReturn, 10, 0);
			}
			break;
			/*获取两个日期内的所有图片文件数据，并删除*/
			case 6:
			{
				Uint32 ui32FileNum = 0;
				ui8Ret = RS_FS_Find(&ui32FileNum, g_szParam1, g_szParam2);
				if (0 == ui8Ret && ui32FileNum > 0)
				{
					Uint32 i = 0;
					Uint32 ui32FileSize;
					char szFileName[MAX_FILE_NAME_LEN];
					Uint32ToBuf(aui8Temp, ui32FileNum);
					send(sockSDFSSvr, aui8Temp, 4, 0);					
					for (; i < ui32FileNum; i++)
					{
						RS_FS_GetNext(szFileName, MAX_FILE_NAME_LEN, i);
						RS_FS_GetFileSize(&ui32FileSize, szFileName);
						if (ui32FileSize > ui32DataBufSize)
						{
							ui32FileSize = ui32DataBufSize;
						}
						RS_FS_Read(pui8DataBuf, ui32DataBufSize, szFileName);
						Uint32ToBuf(aui8Temp, ui32FileSize + MAX_FILE_NAME_LEN);
						send(sockSDFSSvr, aui8Temp, 4, 0);
						send(sockSDFSSvr, szFileName, MAX_FILE_NAME_LEN, 0); 
						send(sockSDFSSvr, pui8DataBuf, ui32FileSize, 0);
						RS_FS_Del(szFileName);

						if (i%100 == 0)
						{
							TSK_sleep(100);
						}
					}
				}
			}
			break;
			/*获取某个图片文件数据，并删除*/
			case 7:
			{
				Uint32 ui32FileSize;
				RS_FS_GetFileSize(&ui32FileSize, g_szParam1);
				if (ui32FileSize > ui32DataBufSize)
				{
					ui32FileSize = ui32DataBufSize;
				}
				RS_FS_Read(pui8DataBuf, ui32DataBufSize, g_szParam1); 
				Uint32ToBuf(aui8Temp, 1);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, ui32FileSize + MAX_FILE_NAME_LEN);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				send(sockSDFSSvr, g_szParam1, MAX_FILE_NAME_LEN, 0);
				send(sockSDFSSvr, pui8DataBuf, ui32FileSize, 0);
				RS_FS_Del(g_szParam1);
			}
			break;
			/*获取相机存储容量*/
			case 8:
			{
				Uint32ToBuf(aui8Temp, 1);
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, RS_FS_GetIdleBlocks());
				send(sockSDFSSvr, aui8Temp, 4, 0);
				Uint32ToBuf(aui8Temp, RS_FS_GetUsedBlocks());
				send(sockSDFSSvr, aui8Temp, 4, 0);
			}
			break;
			default:
			break;
		}
		fdClose(sockSDFSSvr);
		g_ui8SDFSCMD = 0;
	}
	
}


















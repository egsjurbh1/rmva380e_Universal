
#ifndef _RS_FS_FILESYS_H
#define _RS_FS_FILESYS_H
#include "EE3_common.h"

int RS_FS_Initialize(Uint32 ui32ReserveSize, Uint32 ui32ImgSize, Uint32 ui32SingleBuffSize, Uint32 ui32BuffNum);

int RS_FS_Uninitialize();

int RS_FS_IsReady();

int RS_FS_Write(Uint8* pui8FileBuf, Uint32 ui32FileSize);

int RS_FS_GetFileSize(Uint32* pui32FileSize, const char* pszFileName);

int RS_FS_Read(Uint8* pui8FileBuf, Uint32 ui32BufSize, const char* pszFileName);

int RS_FS_Del(const char* pszFileName);

int RS_FS_DelByDate(const char* pszDateBegin, const char* pszDateEnd);

int RS_FS_Find(Uint32* pui32FileNum, const char* pszDateBegin, const char* pszDateEnd);

int RS_FS_GetNext(char* pszBuf, Uint32 ui32BufSize, Uint32 ui32Index);

int RS_FS_GetTotalBlocks();

int RS_FS_GetUsedBlocks();

int RS_FS_GetIdleBlocks();

int RS_FS_GetIdleBlocksABC(Uint32 *pui32CountA, Uint32 *pui32CountB, Uint32 *pui32CountC);

int RS_FS_GetTotalBlocksABC(Uint32 *pui32CountA, Uint32 *pui32CountB, Uint32 *pui32CountC);

#endif

/*
 *  Copyright 2007 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 *  @(#) TCP/IP_Network_Developers_Kit 1.93.00.09 08-16-2007 (ndk-c09)
 */
//----------------------------------------------------------------------
//  Ip Stack
//----------------------------------------------------------------------
//  File: rawif.h
//
//  RAW Interface Functions
//
// Author: Michael A. Denio
// Copyright 1999 by Texas Instruments Inc.
//----------------------------------------------------------------------

#ifndef _RAWIF_INC_
#define _RAWIF_INC_

//
// UDP Statistics
//
typedef struct _rawstat {
        UINT32  RcvTotal;       // total packets received
        UINT32  RcvFull;        // unable to take more data
        UINT32  SndTotal;       // total packets sent
        UINT32  SndNoPacket;    // unable to allocate packet
        } RAWSTATS;

extern RAWSTATS raws;

//
// RAW Functions
//
_extern void  RawInput( PBM_Pkt *pPkt );
_extern int   RawOutput( HANDLE h, UINT8 *pBuf, INT32 sz, INT32 *prsz );

#endif



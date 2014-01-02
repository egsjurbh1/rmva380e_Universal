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
//  File: udpif.h
//
//  UDP Interface Functions
//
// Author: Michael A. Denio
// Copyright 1999 by Texas Instruments Inc.
//----------------------------------------------------------------------

#ifndef _UDPIF_INC_
#define _UDPIF_INC_

//
// UDP Statistics
//
typedef struct _udpstat {
        UINT32  RcvTotal;       // total packets received
        UINT32  RcvShort;       // packet shorter than header
        UINT32  RcvBadLen;      // length larger than packet size
        UINT32  RcvBadSum;      // packets received with chksum errs
        UINT32  RcvFull;        // packets received with socket full
        UINT32  RcvNoPort;      // no socket on destination port
        UINT32  RcvNoPortB;     // NoPort on multicast/broadcast packet
        UINT32  SndTotal;       // total packets sent
        UINT32  SndNoPacket;    // unable to allocate packet
        } UDPSTATS;

extern UDPSTATS udps;
extern PSEUDO   upseudo;

//
// UDP Functions
//
_extern void  UdpInput( PBM_Pkt *pPkt );
_extern int   UdpOutput( HANDLE h, UINT8 *pBuf, INT32 sz, INT32 *prsz );
_extern void  UdpChecksum( UDPHDR * );

#endif



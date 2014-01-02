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
//  File: tcpif.h
//
//  TCP Interface Functions
//
// Author: Michael A. Denio
// Copyright 1999 by Texas Instruments Inc.
//----------------------------------------------------------------------

#ifndef _TCPIF_INC_
#define _TCPIF_INC_

//
// TCP Statistics
//
typedef struct _tcpstat {
        UINT32  ConnAttempt;          // Connection Attempts
        UINT32  ConnDrops;            // Dropped Connection Attempts
        UINT32  Accepts;              // Connections Accepted
        UINT32  Drops;                // Dropped Connections
        UINT32  PersistTimeout;       // Times Persist Timer Expired
        UINT32  KeepTimeout;          // Times Keep or Connect Time Expired
        UINT32  RexmtTimeout;         // ReTx Timeouts
        UINT32  KeepProbe;            // Number of Keep probes sent
        UINT32  KeepDrops;            // Connections dropped in Keep
        UINT32  TimeoutDrops;         // Connections dropped in ReTx
        UINT32  Connects;             // connections established
        UINT32  RttUpdated;           // times RTT timer was successful
        UINT32  DelAck;               // delayed acks sent

        UINT32  SndTotal;             // total packets sent
        UINT32  SndProbe;             // window probes sent
        UINT32  SndPack;              // data packets sent
        UINT32  SndByte;              // data bytes sent
        UINT32  SndRexmitPack;        // data packets retransmitted
        UINT32  SndRexmitByte;        // data bytes retransmitted
        UINT32  SndAcks;              // ack-only packets sent
        UINT32  SndCtrl;              // control (SYN|FIN|RST) packets sent
        UINT32  SndUrg;               // packets sent with URG only
        UINT32  SndWinUp;             // window update-only packets sent
        UINT32  SndNoBufs;            // Out of buffers in TcpOutput
        UINT32  SegsTimed;            // segs where we tried to get rtt

        UINT32  RcvTotal;             // total packets received
        UINT32  RcvShort;             // packets received too short
        UINT32  RcvHdrSize;           // bad TCP header size
        UINT32  RcvBadSum;            // packets received with chksum errs
        UINT32  RcvDupPack;           // duplicate-only packets received
        UINT32  RcvDupByte;           // duplicate-only bytes received
        UINT32  RcvPartDupPack;       // partial duplicate packets received
        UINT32  RcvPartDupByte;       // dup bytes from partial dup packets
        UINT32  RcvAfterClose;        // packets rcvd after "close"
        UINT32  RcvAfterWinPack;      // packets with data past our window
        UINT32  RcvAfterWinByte;      // bytes received past our window
        UINT32  RcvWinProbe;          // rcvd window probe packets
        UINT32  RcvDupAck;            // rcvd duplicate acks
        UINT32  RcvAckTooMuch;        // rcvd acks for unsent data
        UINT32  RcvAckPack;           // rcvd ack packets
        UINT32  RcvAckByte;           // bytes acked by rcvd acks
        UINT32  RcvWinUpd;            // rcvd window update packets
        UINT32  RcvPack;              // packets received in sequence
        UINT32  RcvByte;              // bytes received in sequence
        UINT32  RcvOOPack;            // packets received out of sequence
        UINT32  RcvOOByte;            // bytes received out of sequence
        } TCPSTATS;

extern TCPSTATS tcps;
extern PSEUDO   tpseudo;

//
// Sequence MACROS
//
#define SEQ_LT(a,b) ((INT32)((a)-(b)) < 0)
#define SEQ_LEQ(a,b) ((INT32)((a)-(b)) <= 0)
#define SEQ_GT(a,b) ((INT32)((a)-(b)) > 0)
#define SEQ_GEQ(a,b) ((INT32)((a)-(b)) >= 0)

//
// TCP Functions
//
_extern void  TcpTimeoutCheck();    // Called every 0.1 sec
_extern void  TcpChecksum( TCPHDR * );
_extern void  TcpInput( PBM_Pkt *pPkt );
_extern void  TcpQuench( HANDLE hTcp );

//
// TCP Protocol Entry Points
//
_extern int   TcpPrAttach( HANDLE h, HANDLE *phTcp );
_extern int   TcpPrDetach( HANDLE h, HANDLE *phTcp, int fatal );
_extern int   TcpPrListen( HANDLE h, HANDLE hTcp );
_extern int   TcpPrConnect( HANDLE h, HANDLE hTcp );
_extern int   TcpPrDisconnect( HANDLE h, HANDLE hTcp );
_extern int   TcpPrRecv( HANDLE h, HANDLE hTcp );
_extern int   TcpPrSend( HANDLE h, HANDLE hTcp, UINT8 *pBuf, INT32 sz, INT32 *prsz );
_extern int   TcpPrSendOOB( HANDLE h, HANDLE hTcp, UINT8 *pBuf, INT32 sz, INT32 *prsz );
_extern void  TcpPrInherit( HANDLE hP, HANDLE hTcpP, HANDLE hC, HANDLE hTcpC );
_extern int   TcpPrSetOption( HANDLE h, HANDLE hTcp, int Prop, void *pbuf, int size );
_extern int   TcpPrGetOption( HANDLE h, HANDLE hTcp, int Prop, void *pbuf, int *psize );
_extern void  TcpPrCtlError( HANDLE h, HANDLE hTcp, uint Code, int Error );
_extern uint  TcpPrGetState( HANDLE h, HANDLE hTcp );

#endif

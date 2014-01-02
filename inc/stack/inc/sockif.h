/*
 *  Copyright 2007 by Texas Instruments Incorporated.
 *  All rights reserved. Property of Texas Instruments Incorporated.
 *  Restricted rights to use, duplicate or disclose this code are
 *  granted through contract.
 *
 *  @(#) TCP/IP_Network_Developers_Kit 1.93.00.09 08-16-2007 (ndk-c09)
 */
/*
 *  ======== sockif.h ========
 *
 *
 */

//----------------------------------------------------------------------
//  Ip Stack
//----------------------------------------------------------------------
//  File: sockif.h
//
//  Sock Object public definitions
//
// Author: Michael A. Denio
// Copyright 1999 by Texas Instruments Inc.
//----------------------------------------------------------------------
#include "fdtif.h"
#include "listlib.h"

#ifndef _SOCKIF_INC_
#define _SOCKIF_INC_

//
// Socket Ephemeral Port Range (Public and Reserved)
//
#define SOCK_USERPORT_FIRST 0xE000      // Local User Ports
#define SOCK_USERPORT_LAST  0xEFFF
#define SOCK_RESPORT_FIRST  0xF000      // Reserved NAT Ports
#define SOCK_RESPORT_LAST   0xFFFE

//
// Protocol Control Commands
//
#define PRC_IFDOWN              0       // interface transition
#define PRC_ROUTEDEAD           1       // select new route if possible ???
#define PRC_QUENCH2             3       // DEC congestion bit says slow down
#define PRC_QUENCH              4       // some one said to slow down
#define PRC_MSGSIZE             5       // message size forced drop
#define PRC_HOSTDEAD            6       // host appears to be down
#define PRC_HOSTUNREACH         7       // deprecated (use PRC_UNREACH_HOST)
#define PRC_UNREACH_NET         8       // no route to network
#define PRC_UNREACH_HOST        9       // no route to host
#define PRC_UNREACH_PROTOCOL    10      // dst says bad protocol
#define PRC_UNREACH_PORT        11      // bad port #
#define PRC_UNREACH_NEEDFRAG    12      // (use PRC_MSGSIZE)
#define PRC_UNREACH_SRCFAIL     13      // source route failed
#define PRC_REDIRECT_NET        14      // net routing redirect
#define PRC_REDIRECT_HOST       15      // host routing redirect
#define PRC_REDIRECT_TOSNET     16      // redirect for type of service & net
#define PRC_REDIRECT_TOSHOST    17      // redirect for tos & host
#define PRC_TIMXCEED_INTRANS    18      // packet lifetime expired in transit
#define PRC_TIMXCEED_REASS      19      // lifetime expired on reass q
#define PRC_PARAMPROB           20      // header incorrect
#define PRC_NCMDS               21

//
// IP Socket Options (IpFlags)
//
#define SOCK_IP_HDRINCL         0x0001  // Include IP Header (raw only)
#define SOCK_IP_OPTIONS         0x0002  // Use supplied IP Options

/* Multicast Socket Record Information:
 *  Multicast requests are now stored on per socket basis; instead of a per
 *  interface basis. This structure is a linked list of multicast records which
 *  are stored on each socket. Sockets could have joined multiple multicast 
 *  groups. */
typedef struct _mcast_sock
{
    LIST_NODE           links;          /* List of multicast socket records */
    struct ip_mreq      mreq;           /* Multicast Request                */
    HANDLE              hIf;            /* Handle to the interface          */
}MCAST_SOCK_REC;

/* Socket Priority 
 *  Each socket can be associated with a specific priority. This priority
 *  can be configured through the setsockopt API. All packets transmitted
 *  through this socket will have the same priority. This is used by the 
 *  VLAN drivers in the system to remark the packets with an appropriate
 *  Layer2 User Priority value. 
 *  By default; there is no priority associated with the socket. */
#define PRIORITY_UNDEFINED 0xFFFF

// Sock Object Structure
typedef struct _sock {
             FILEDESC   fd;             // File descriptor header

             uint       Family;         // Address Family as AF_ in socket.h
             uint       SockType;       // Type as SOCK_ in socket.h
             uint       Protocol;       // IP Protocol: TCP, UDP, ICMP, etc.

             uint       IpFlags;        // IP Protocol Options (raw only)
             uint       IpTtl;          // IP TTL
             uint       IpTos;          // IP TOS
             uint       IpOptSize;      // IP Header Options Size
             UINT8      IpOptions[40];  // IP Header Options

             UINT16     SockPriority;   // Socket Priority.
             uint       SockProt;       // Socket Protocol Handler
#define SOCKPROT_NONE   0
#define SOCKPROT_TCP    1
#define SOCKPROT_UDP    2
#define SOCKPROT_RAW    3

             uint       OptionFlags;    // SO_ options as defined in socket.h
             uint       StateFlags;     // SS_ flags as defined below
             UINT32     dwLingerTime;   // Time used when SO_LINGER set

             // Protocol Control
             struct _sock *pProtNext;   // Pointer to next in protocol list
             struct _sock *pProtPrev;   // Pointer to prev in protocol list
             IPN        FIP;            // Foreign IP address
             uint       FPort;          // Foreign Port
             IPN        LIP;            // Local IP address (NULL for wildcard)
             uint       LPort;          // Local Port (NULL if not bound)
             IPN        BIP;            // Bound IP address (SS_ADDR only)
             uint       BPort;          // Bound Port (SS_ADDR only)
             HANDLE     hTP;            // Handle to protocol specific data
             HANDLE     hRoute;         // Handle to cached route
             HANDLE     hIFTx;          // Handle to Default IF for transmit

             // Connection State Stuff
             struct _sock *pParent;     // Pointer back to accept socket
             struct _sock *pPrevQ;      // Prev socket in pend/ready queue
             struct _sock *pPending;    // Pending connection sockets
             struct _sock *pReady;      // Ready connected sockets
             uint       ConnMax;        // Max pending/ready connections
             uint       ConnTotal;      // Total connections

             // Read/Write Stuff
             int        ErrorPending;   // Error returned on next socket call
             INT32      OOBMark;        // Out of band info mark
             uint       OOBData;        // Out of band info data
             UINT32     RxTimeout;      // Timeout for Rx IO on stream
             UINT32     TxTimeout;      // Timeout for Tx IO on stream
             UINT32     RxBufSize;      // Rx Buffer Size
             UINT32     TxBufSize;      // Tx Buffer Size
             HANDLE     hSBRx;          // Rx Buffer
             HANDLE     hSBTx;          // Tx Buffer

             MCAST_SOCK_REC* pMcastList; // List of multicast addresses on the socket.

             TimestampFxn pTimestampFxn; // Callout function pointer to
                                         // timestamp TX 
        } SOCK;

//
// Socket Protocol Block
//
typedef struct _sockpcb {
             IPN        IPAddrLocal;    // Local IP Address
             uint       PortLocal;      // Local IP Port
             IPN        IPAddrForeign;  // Foreign IP Address
             uint       PortForeign;    // Foreign IP Port
             uint       State;          // Socket State (protocol dependent)
        } SOCKPCB;

//
// Socket Access Functions
//

//------------------------------------------------------------------------
// General Access Functions (called from upper layers)
//
_extern int    SockNew( int Family, int Type, int Protocol,
                        int RxBufSize, int TxBufSize, HANDLE *phSock );
_extern int    SockClose( HANDLE hSock );

_extern int    SockCheck( HANDLE hSock, int IoType );
#define  SOCK_READ       0
#define  SOCK_WRITE      1
#define  SOCK_EXCEPT     2

_extern int    SockStatus( HANDLE hSock, int request, int *results );

_extern int    SockSet(HANDLE hSock, int Type, int Prop, void *pbuf, int size);
_extern int    SockGet(HANDLE hSock, int Type, int Prop, void *pbuf, int *psize);

_extern int    SockShutdown( HANDLE hSock, int how );

_extern int    SockConnect( HANDLE hSock, PSA pName );
_extern int    SockDisconnect( HANDLE hSock );
_extern int    SockBind( HANDLE hSock, PSA pName );

_extern int    SockGetName( HANDLE hSock, PSA pSockName, PSA pPeerName );

_extern int    SockListen( HANDLE hSock, int maxcon );
_extern int    SockAccept( HANDLE hSock, HANDLE *phSock );

_extern int    SockRecv( HANDLE hSock, INT8 *pBuf, INT32 size,
                        int flags, PSA pPeer, INT32 *pRetSize );
_extern int    SockSend( HANDLE hSock, INT8 *pBuf, INT32 size,
                        int flags, INT32 *pRetSize );
_extern int    SockRecvNC( HANDLE hSock, int flags, PSA pPeer, PBM_Pkt **ppPkt );

_extern int    SockGetPcb( uint SockProt, uint BufSize, UINT8 *pBuf );
_extern void SockCleanPcb (uint SockProt, IPN IPAddress);

//------------------------------------------------------------------------
// PCB Related Socket Access Functions (called from stack protocols)
//
_extern int      SockPcbAttach( HANDLE hSock );
_extern int      SockPcbDetach( HANDLE hSock );
_extern void     SockPcbCleanup();
_extern int      SockPcbBind( HANDLE hSock, IPN IP, uint Port );
_extern int      SockPcbConnect( HANDLE hSock, IPN IP, uint Port );
_extern HANDLE   SockPcbResolve( uint SockProt, IPN LIP, uint LPort,
                                IPN FIP, uint FPort, uint Match );
#define SOCK_RESOLVE_BEST       1       // Return Best Match
#define SOCK_RESOLVE_EXACT      2       // Return Exact Match
#define SOCK_RESOLVE_SPAWN      3       // Return exact, or spawn on best match
_extern HANDLE   SockPcbResolveChain( HANDLE hSock, uint wSockProt, uint Prot,
                                IPN LIP, uint LPort, IPN FIP, uint FPort );
_extern void   SockPcbCtlError( uint Code, IPHDR *pIpHdr );
_extern void   SockPcbRtChange( HANDLE hRt );

//------------------------------------------------------------------------
// Low-level Access Functions (called from stack protocols)
//
_extern int    SockNotify( HANDLE hSock, int Notification );
#define SOCK_NOTIFY_CONNECT     1
#define SOCK_NOTIFY_RCVACK      2
#define SOCK_NOTIFY_RCVDATA     3
#define SOCK_NOTIFY_RCVFIN      4
#define SOCK_NOTIFY_DISCONNECT  5
#define SOCK_NOTIFY_CLOSED      6
#define SOCK_NOTIFY_ERROR       7

_extern void   SockSetOOBMark( HANDLE hSock, INT32 OOBMark );
_extern void   SockSetOOBData( HANDLE hSock, UINT8 OOBData );
_extern void   SockSpawnAbort( HANDLE hSock );
_extern HANDLE SockValidateRoute( HANDLE hSock );
_extern PBM_Pkt *SockCreatePacket( HANDLE hSock, uint Size );

//
// Low Level Object Interface
//

#ifdef _STRONG_CHECKING
_extern uint   SockGetProtocol( HANDLE h );
_extern uint   SockGetIpHdrSize( HANDLE h );
_extern HANDLE SockGetTx( HANDLE h );
_extern HANDLE SockGetRx( HANDLE h );
_extern HANDLE SockGetTP( HANDLE h );
_extern IPN    SockGetLIP( HANDLE h );
_extern IPN    SockGetFIP( HANDLE h );
_extern uint   SockGetLPort( HANDLE h );
_extern uint   SockGetFPort( HANDLE h );
_extern HANDLE SockGetRoute( HANDLE h );
_extern HANDLE SockGetIFTx( HANDLE h );
_extern uint   SockGetOptionFlags( HANDLE h );
_extern void   SockSetError( HANDLE h, int Error );
#else
#define SockGetProtocol( h )     (((SOCK *)h)->Protocol)
#define SockGetIpHdrSize( h )    ((((SOCK *)h)->IpFlags & IP_HDRINCL) ? 0 : (IPHDR_SIZE + (((SOCK *)h)->IpOptSize)))
#define SockGetTx( h )           (((SOCK *)h)->hSBTx)
#define SockGetRx( h )           (((SOCK *)h)->hSBRx)
#define SockGetTP( h )           (((SOCK *)h)->hTP)
#define SockGetLIP( h )          (((SOCK *)h)->LIP)
#define SockGetFIP( h )          (((SOCK *)h)->FIP)
#define SockGetLPort( h )        (((SOCK *)h)->LPort)
#define SockGetFPort( h )        (((SOCK *)h)->FPort)
#define SockGetRoute( h )        (((SOCK *)h)->hRoute)
#define SockGetIFTx( h )         (((SOCK *)h)->hIFTx)
#define SockGetOptionFlags( h )  (((SOCK *)h)->OptionFlags)
#define SockSetError( h, e )     (((SOCK *)h)->ErrorPending=e)
#endif

#endif

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
//  File: netdf.h
//
//  Basic Network Data Formats
//
// Author: Michael A. Denio
// Copyright 1999 by Texas Instruments Inc.
//----------------------------------------------------------------------
#ifndef _C_NETDF_INC
#define _C_NETDF_INC  /* #defined if this .h file has been included */

//----------------
//
// Ethernet Header
//
#define ETHHDR_SIZE     14

typedef struct {
                UINT8   DstMac[6];
                UINT8   SrcMac[6];
                UINT16  Type;
               } ETHHDR;

//----------------
//
// VLAN Header
//
#define VLANHDR_SIZE     4

typedef struct {
                UINT16  TCI;
                UINT16  EncapProtocol;
               } VLANHDR;

//-----------------------------------------------------------------------
//
// Ethernet ARP Protocol Header
//

#define ARPHDR_SIZE     28

typedef struct {
                UINT16   HardType;
                UINT16   ProtocolType;
                UINT8    HardSize;
                UINT8    ProtocolSize;
                UINT16   Op;
                UINT8    SrcAddr[6];
                UINT8    IPSrc[4];
                UINT8    DstAddr[6];
                UINT8    IPDst[4];
               } ARPHDR;

//----------------------------------------------------
//
// IP HEADER
//

#define IPHDR_SIZE      20

typedef struct {
                UINT8    VerLen;
                UINT8    Tos;
                UINT16   TotalLen;
                UINT16   Id;
                UINT16   FlagOff;
                UINT8    Ttl;
                UINT8    Protocol;
                UINT16   Checksum;
                IPN      IPSrc;
                IPN      IPDst;
                UINT8    Options[1];
               } IPHDR;

//----------------------------------------------------
//
// ICMP HEADER
//

#define ICMPHDR_SIZE    4

typedef struct {
                UINT8    Type;
                UINT8    Code;
                UINT16   Checksum;
                UINT8    Data[1];
               } ICMPHDR;

#define ICMPREQHDR_SIZE 4

typedef struct {
                UINT16    Id;
                UINT16    Seq;
                UINT8     Data[1];
               } ICMPREQHDR;

#define ICMPRTAHDR_SIZE 12

typedef struct {
                UINT8    NumAddr;
                UINT8    Size;
                UINT16   Lifetime;
                struct _rta {
                             IPN     IPAddr;
                             INT32   Pref;
                        } rta[1];
               } ICMPRTAHDR;


//----------------------------------------------------
//
// IGMP HEADER
//

#define IGMPHDR_SIZE    8

typedef struct {
                UINT8    VerType;
                UINT8    MaxTime;
                UINT16   Checksum;
                IPN      IpAddr;
               } IGMPHDR;


//----------------------------------------------------
//
// TCP HEADER
//

#define TCPHDR_SIZE     20

typedef struct {
                UINT16   SrcPort;
                UINT16   DstPort;
                UINT32   Seq;
                UINT32   Ack;
                UINT8    HdrLen;
                UINT8    Flags;
                UINT16   WindowSize;
                UINT16   TCPChecksum;
                UINT16   UrgPtr;
                UINT8    Options[1];
               } TCPHDR;

//----------------------------------------------------
//
// UDP HEADER
//
#define UDPHDR_SIZE     8

typedef struct {
                UINT16   SrcPort;
                UINT16   DstPort;
                UINT16   Length;
                UINT16   UDPChecksum;
               } UDPHDR;

// Pseudo Header for Checksum
typedef struct {
                IPN     IPSrc;
                IPN     IPDst;
                UINT16  Length;
                UINT8   Null;
                UINT8   Protocol;
                } PSEUDO;

#endif

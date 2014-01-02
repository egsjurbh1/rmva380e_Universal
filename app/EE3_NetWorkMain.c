/**
 * \file      	EE3_NetWorkMain.c 
 * \author    	LQ
 * \version   	0.01
 * \date      	2012.11.30
 * \brief     	网络初始化模块
 * \update      
**/

#include "EE3_common.h"
#include <std.h>
#include <stdio.h>
#include <c62.h>
#include <netmain.h>

//---------------------------------------------------------------------------
// Title String
//
char *VerStr = "TCP/IP Stack 'EE3_Series_NetWork!' Application\n";

// Our NETCTRL callback functions
static void   EE3_NetworkOpen();
static void   EE3_NetworkClose();
static void   EE3_NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd );

// Fun reporting function
static void   ServiceReport( uint Item, uint Status, uint Report, HANDLE hCfgEntry );

extern void rtspMainThread();
static HANDLE hRtspMain=0;
unsigned int g_IPAddrCurUse = 0;

/**************************************************************************/
//TCP 子任务
extern void bootsrv();				//远程更新DSP程序
extern void GeneralCmdSrv();		//命令交互
extern void SendTrgDataPort();		//上传触发模式数据
extern void SendFconDataPort();		//上传全分辨率连续模式的MJPG图像
extern void SendLiveStreamDataPort();//上传实时H264视频流
extern void SendSDFSDataPort();//	SD卡传输模块
extern void TaskCameraInfoNetServe();
extern void SendLiveStreamDataPort_Another();//上传实时H264视频流
//自主研发通信线程
extern void ReceBpStartSemPort();       //接收控制端控制信号
extern void UDPlistensendPort();       //平台（配置工具）UDP广播通信
extern void ReceDecOrderSemPort();      //平台控制信号通信
extern void SendFcontoDecPort();         //向平台发送图像数据
extern void SendRoadInfPort();          //向服务器发送数据
/**************************************************************************/
// System Task Code
static HANDLE hBoot=0;
static HANDLE hSend_Trg=0;
static HANDLE hSend_Con=0;
static HANDLE hGnlCmd=0;
static HANDLE hSend_LiveStream=0;
static HANDLE hSDFSSend=0;
static HANDLE hCameraInfo =0;
static HANDLE hSend_LiveStream_Another=0;
static HANDLE hRece_BpSem=0; //接收后台
static HANDLE hUDP_Sem=0; //new,UDP信号
static HANDLE hReceDecO_Sem=0; //new,平台控制
static HANDLE hSend_FcontoDec_Sem=0; //new,平台发送
static HANDLE hSend_roadinfo_Sem=0;//new,服务器发送
/**************************************************************************/
//文件局部变量声明
static char *pcHostName    = "EagleEye3"; 
static char *pcLocalIPMask = "255.255.255.0";    // Not used when using DHCP
static char *pcGatewayIP   = "192.168.0.1";      // Not used when using DHCP
static char *pcDomainName  = "demo.net";         // Not used when using DHCP
static char *pcDNSServer   = "0.0.0.0";          // Used when set to anything but zero
static int   iMainConfigValid  = 0;
/**************************************************************************/

// Main Thread
//
extern void Roseek_SysParamInit();
int EE3_NetWorkMain()
{
	int					rc;
	HANDLE				hCfg;
	unsigned int		IPAddr;

	//配置IP地址
	IPAddr = Roseek_Read_IP_Address();
	//IPAddr = 0xf701a8c0;
	g_IPAddrCurUse = IPAddr;

	if( IPAddr == 0) {
		// IP address is invalid, so enable config-by-ping function
		iMainConfigValid = 0;
	}
	else {
		// IP address is invalid, disable config-by-ping function
		iMainConfigValid = 1;
	}

	rc = NC_SystemOpen( NC_PRIORITY_LOW, NC_OPMODE_INTERRUPT );
	if( rc )
	{
		printf("NC_SystemOpen Failed (%d)\n",rc);
		for(;;);
	}

	// Print out our banner
	printf(VerStr);

	// Create a new configuration
	hCfg = CfgNew();
	if( !hCfg )
	{
		printf("Unable to create configuration\n");
		goto main_exit;
	}

	// We better validate the length of the supplied names
	if( strlen( pcDomainName ) >= CFG_DOMAIN_MAX ||
		strlen( pcHostName ) >= CFG_HOSTNAME_MAX )
	{
		printf("Names too long\n");
		goto main_exit;
	}

	// Add our global hostname to hCfg (to be claimed in all connected domains)
	CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_HOSTNAME, 0,
				strlen(pcHostName), (UINT8 *)pcHostName, 0 );

	// If the IP address is specified, manually configure IP and Gateway
	if( iMainConfigValid )
	{
		CI_IPNET NA;
        CI_ROUTE RT;
        IPN      IPTmp;
        // Setup manual IP address
		bzero( &NA, sizeof(NA) );
		NA.IPAddr  = IPAddr;
		NA.IPMask  = inet_addr(pcLocalIPMask);
		strcpy( NA.Domain, pcDomainName );
		NA.NetType = 0;
        // Add the address to interface 1
		CfgAddEntry( hCfg, CFGTAG_IPNET, 1, 0,
					sizeof(CI_IPNET), (UINT8 *)&NA, 0 );

		// Add the default gateway. Since it is the default, the
		// destination address and mask are both zero (we go ahead
		// and show the assignment for clarity).
		bzero( &RT, sizeof(RT) );
		RT.IPDestAddr = 0;
		RT.IPDestMask = 0;
		RT.IPGateAddr = inet_addr(pcGatewayIP);
		// Add the route
		CfgAddEntry( hCfg, CFGTAG_ROUTE, 0, 0,
						sizeof(CI_ROUTE), (UINT8 *)&RT, 0 );

		// Manually add the DNS server when specified
		IPTmp = inet_addr(pcDNSServer);
		if( IPTmp )
		CfgAddEntry( hCfg, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
					0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
	}
	// Else we specify DHCP
	else
	{
		CI_SERVICE_DHCPC dhcpc;

		// Specify DHCP Service on IF-1
		bzero( &dhcpc, sizeof(dhcpc) );
		dhcpc.cisargs.Mode   = CIS_FLG_IFIDXVALID;
		dhcpc.cisargs.IfIdx  = 1;
		dhcpc.cisargs.pCbSrv = &ServiceReport;
		CfgAddEntry( hCfg, CFGTAG_SERVICE, CFGITEM_SERVICE_DHCPCLIENT, 0,
					sizeof(dhcpc), (UINT8 *)&dhcpc, 0 );
	}

    // We don't want to see debug messages less than WARNINGS
    rc = DBG_WARN;
    CfgAddEntry( hCfg, CFGTAG_OS, CFGITEM_OS_DBGPRINTLEVEL,
                 CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

	//
	// This code sets up the TCP and UDP buffer sizes
	// (Note 8192 is actually the default. This code is here to
	// illustrate how the buffer and limit sizes are configured.)
	//
    // TCP Transmit buffer size
	rc = 8192;
	CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPTXBUF,
				CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

	// TCP Receive buffer size (copy mode)
	rc = 8192;
	CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXBUF,
				CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

	// TCP Receive limit (non-copy mode)
	rc = 8192;
	CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKTCPRXLIMIT,
				CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

	// UDP Receive limit
	rc = 8192;
	CfgAddEntry( hCfg, CFGTAG_IP, CFGITEM_IP_SOCKUDPRXLIMIT,
				CFG_ADDMODE_UNIQUE, sizeof(uint), (UINT8 *)&rc, 0 );

	//
	// Boot the system using this configuration
	//
	// We keep booting until the function returns 0. This allows
	// us to have a "reboot" command.
	//
	do
	{
		rc = NC_NetStart( hCfg, EE3_NetworkOpen, EE3_NetworkClose, EE3_NetworkIPAddr );
	} while( rc > 0 );

	// Delete Configuration
	CfgFree( hCfg );

	// Close the OS
main_exit:
	NC_SystemClose();
	return(0);
}

//
// EE3_NetworkOpen
// This function is called after the configuration has booted
//
static void EE3_NetworkOpen()
{
    // Create our local server
	hSend_Con	= TaskCreate( SendFconDataPort, "SendConDataPort", 5, 0x1400, 0, 0, 0 );
	hSend_Trg = TaskCreate( SendTrgDataPort, "SendTrgDataPort", 5, 0x1400, 0, 0, 0 );
	hGnlCmd = TaskCreate( GeneralCmdSrv, "GeneralCmdSrv", 5, 0x1400, 0, 0, 0 );

	// Create our local servers
	hBoot = TaskCreate( bootsrv, "BootSrv", 5, 0x1400, 0, 0, 0 );

	hSend_LiveStream = TaskCreate( SendLiveStreamDataPort, "SendLiveStreamDataPort", 5, 0x1400, 0, 0, 0 );
	hSDFSSend = TaskCreate( SendSDFSDataPort, "SendDataPort", 6, 0x1400, 0, 0, 0 );
	hRtspMain = TaskCreate( rtspMainThread, "rtspMainThread", 6, 0x1400, 0, 0, 0 );
	hCameraInfo = TaskCreate( TaskCameraInfoNetServe, "TaskCameraInfoNetServe", 5, 0x1400, 0, 0, 0 );
	hSend_LiveStream_Another = TaskCreate( SendLiveStreamDataPort_Another, "SendLiveStreamDataPor_Anothert", 5, 0x1400, 0, 0, 0 );

	//Creat by L.Q.
	hRece_BpSem=TaskCreate( ReceBpStartSemPort,"ReceBpStartSemPort",4,0x1400,0,0,0); //优先级原始为4、5
	hUDP_Sem=TaskCreate( UDPlistensendPort,"UDPlistensendPort",4,0x1400,0,0,0); //4
	hReceDecO_Sem=TaskCreate( ReceDecOrderSemPort,"ReceDecOrderSemPort",4,0x1400,0,0,0); //4
	hSend_FcontoDec_Sem=TaskCreate( SendFcontoDecPort,"SendFcontoDecPort",4,0x1400,0,0,0); //4
	hSend_roadinfo_Sem=TaskCreate( SendRoadInfPort,"SendRoadInfPort",4,0x1400,0,0,0); //4
}

//
// EE3_NetworkClose
//
// This function is called when the network is shutting down,
// or when it no longer has any IP addresses assigned to it.
//
static void EE3_NetworkClose()
{

	fdCloseSession( hSend_Trg );
	TaskDestroy( hSend_Trg );

	fdCloseSession( hSend_Con );
	TaskDestroy( hSend_Con );

	fdCloseSession( hGnlCmd );
	TaskDestroy( hGnlCmd );

	fdCloseSession( hBoot );
	TaskDestroy( hBoot );

	fdCloseSession( hSend_LiveStream );
	TaskDestroy( hSend_LiveStream );

	fdCloseSession(hSDFSSend);
	TaskDestroy(hSDFSSend);

	fdCloseSession(hRtspMain);
	TaskDestroy(hRtspMain);


	fdCloseSession(hCameraInfo);
	TaskDestroy(hCameraInfo);

	fdCloseSession( hSend_LiveStream_Another );
	TaskDestroy( hSend_LiveStream_Another );

	//自主网络线程

	fdCloseSession( hRece_BpSem );
	TaskDestroy( hRece_BpSem );

	fdCloseSession( hUDP_Sem );
	TaskDestroy( hUDP_Sem );

	fdCloseSession( hReceDecO_Sem );
	TaskDestroy( hReceDecO_Sem );

	fdCloseSession( hSend_FcontoDec_Sem );
	TaskDestroy( hSend_FcontoDec_Sem );

	fdCloseSession( hSend_roadinfo_Sem );
	TaskDestroy( hSend_roadinfo_Sem );
}


//
// EE3_NetworkIPAddr
//
// This function is called whenever an IP address binding is
// added or removed from the system.
//
static void EE3_NetworkIPAddr( IPN IPAddr, uint IfIdx, uint fAdd )
{
    IPN IPTmp;

    if( fAdd ){
        printf("Network Added: ");
	}
    else
        printf("Network Removed: ");

    // Print a message
    IPTmp = ntohl( IPAddr );
    printf("If-%d:%d.%d.%d.%d\n", IfIdx,
            (UINT8)(IPTmp>>24)&0xFF, (UINT8)(IPTmp>>16)&0xFF,
            (UINT8)(IPTmp>>8)&0xFF, (UINT8)IPTmp&0xFF );
//	Roseek_SysParamInit();

}

//
// Service Status Reports
//
// Here's a quick example of using service status updates
//
static char *TaskName[]  = { "Telnet","HTTP","NAT","DHCPS","DHCPC","DNS" };
static char *ReportStr[] = { "","Running","Updated","Complete","Fault" };
static char *StatusStr[] = { "Disabled","Waiting","IPTerm","Failed","Enabled" };
static void ServiceReport( uint Item, uint Status, uint Report, HANDLE h )
{
    printf( "Service Status: %-9s: %-9s: %-9s: %03d\n",
            TaskName[Item-1], StatusStr[Status],
            ReportStr[Report/256], Report&0xFF );

    //
    // Example of adding to the DHCP configuration space
    //
    // When using the DHCP client, the client has full control over access
    // to the first 256 entries in the CFGTAG_SYSINFO space.
    //
    // Note that the DHCP client will erase all CFGTAG_SYSINFO tags except
    // CFGITEM_DHCP_HOSTNAME. If the application needs to keep manual
    // entries in the DHCP tag range, then the code to maintain them should
    // be placed here.
    //
    // Here, we want to manually add a DNS server to the configuration, but
    // we can only do it once DHCP has finished its programming.
    //
    if( Item == CFGITEM_SERVICE_DHCPCLIENT &&
        Status == CIS_SRV_STATUS_ENABLED &&
        (Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPADD) ||
         Report == (NETTOOLS_STAT_RUNNING|DHCPCODE_IPRENEW)) )
    {
        IPN IPTmp;

        // Manually add the DNS server when specified
        IPTmp = inet_addr(pcDNSServer);
        if( IPTmp )
            CfgAddEntry( 0, CFGTAG_SYSINFO, CFGITEM_DHCP_DOMAINNAMESERVER,
                         0, sizeof(IPTmp), (UINT8 *)&IPTmp, 0 );
    }
}


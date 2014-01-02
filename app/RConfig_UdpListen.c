/**
 * \file      	RConfig_UdpListen.c
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.1.4
 * \date      	2013.11.24
 * \brief     	远程配置功能——UDP侦听远程配置工具的网络线程
 * \update      
**/

#include "EE3_common.h"
/**************************************************************************/

/**************************************************************************/
//外部变量
extern volatile    struct  sockaddr g_ConfigSetaddr;
extern volatile    int     bTextConnect;
extern volatile    int     bTextCount;
extern volatile    int     bTextRecev;
/**************************************************************************/

/**
 * \function 	UDPlistensendPort
 * \brief    	使用UDP协议实现广播侦听远程配置工具，并主动连接将相机IP地址告知
 * \			默认作为服务端，Port = 4502;主动连接平台时，作为客户端。
 * \
**/	
void UDPlistensendPort()
{
	SOCKET	sockUDPsendListen,sockTCPsendIP;
	int		clilen,len;
	int     label;
	struct	sockaddr_in sin,connaddr, clientaddr; //保存客户的地址信息
	struct  sockaddr pcliaddr;
	ConversationHead replySem;

	//为当前任务配置运行环境
	fdOpenSession( TaskSelf() );

	//创建侦听套接字socket对象
	sockUDPsendListen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//创建服务器监听socket,UDP协议
	if( sockUDPsendListen == INVALID_SOCKET )
	{
		Roseek_Reset();//如果创建侦听对象失败，重启相机
	}
	bzero( &sin, sizeof(struct sockaddr_in) );
	sin.sin_family	= AF_INET; 
	sin.sin_addr.s_addr = INADDR_ANY;//自动获取本机地址
	sin.sin_len	= sizeof( sin );
	sin.sin_port= htons( UDPPORT );//指明连接服务器的端口号
	//sockUDPsendListen绑定
	if ( bind( sockUDPsendListen, (PSA) &sin, sizeof(sin) ) < 0 )
	{
		Roseek_Reset();//如果绑定失败，重启相机
	}

	//迭代接受控制消息
	while(1)
	{
		label = 10; 
	    clilen = sizeof(struct sockaddr);		
		//开始接收数据
		bTextRecev=recvfrom(sockUDPsendListen, (char *)(&replySem), sizeof(ConversationHead), 0, (PSA)&pcliaddr,&clilen);			
		if( bTextRecev < sizeof(ConversationHead) )
		{
			continue;
		}
		//存储配置工具的IP
		g_ConfigSetaddr= pcliaddr;	
					
		//作为客户端，创建TCP协议连接远程平台，端口4502，告知相机的IP
		sockTCPsendIP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建客户端socket.TCP协议
		if( sockTCPsendIP == INVALID_SOCKET )
		{
		    Roseek_Reset();//如果创建侦听对象失败，重启相机
	    }
		bzero( &connaddr,sizeof(struct sockaddr_in));
		connaddr.sin_family = AF_INET;
		connaddr.sin_addr.s_addr = INADDR_ANY;
		connaddr.sin_len = sizeof( connaddr );
		connaddr.sin_port = htons(0);
		if (bind(sockTCPsendIP, (PSA) &connaddr,sizeof(connaddr)))
		{
			Roseek_Reset();//如果绑定失败，重启相机
		} 
		bzero(&clientaddr, sizeof(struct sockaddr_in));
		clientaddr.sin_family = AF_INET;
		clientaddr.sin_addr = ((struct sockaddr_in*) &pcliaddr)->sin_addr;
		clientaddr.sin_len = sizeof( clientaddr );
		clientaddr.sin_port = htons(UDPPORT);		
		if(connect( sockTCPsendIP, (PSA)&clientaddr, clientaddr.sin_len)<0)
		{
			bTextConnect = 1;
			++bTextCount;
			continue;
		}
		else
		{
			bTextConnect = 0;
			if(replySem.command==UDPCON)
			{
				//发送信息1
				replySem.command = BROADCAST1;
				replySem.bufsize = DEVICETYPE;//设备类型参数
		      	len=send( sockTCPsendIP,&replySem, sizeof(replySem), 0 );
		      	//发送信息2
		      	replySem.command = UDPSETTIME;
		      	replySem.bufsize = label;
		      	len=send( sockTCPsendIP,&replySem, sizeof(replySem), 0);
		      	fdClose(sockTCPsendIP);
				if(len<0)
				{
					continue;
				}
			}
		}			

	}//迭代循环

}

/**
 * \file      	Rconfig_SendYuvData.c
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.0.6
 * \date      	2013.11.24
 * \brief     	远程配置功能——YUV图像数据发送至配置工具的网络线程
 * \update      
**/				

#include "EE3_common.h"
/**************************************************************************/
//外部变量
extern ITS_ImgBufStruct      g_ImgBufofFrontS;//图像原始数据缓存结构体
extern volatile	Bool	g_bIsFcontoDecClientConnect;
extern volatile Bool    g_bIsSendPictoDec;
extern volatile    struct  sockaddr g_ConfigSetaddr;
/**************************************************************************/

/**
 * \function 	SendFcontoDecPort
 * \brief    	使用TCP/IP协议，向配置工具发送YUV422格式的原始图像数据
 * \			作为客户端，Port = 4501		
 * \
**/	
void SendFcontoDecPort()
{
	SOCKET	sockTCPsendVideo;
	int		cnt;
	struct	sockaddr_in connaddr,clientaddr;	
	
	while(1)
	{
		SEM_pend(&sem_SendVideoReady,SYS_FOREVER);			    			    
	    //为当前任务配置运行环境
		fdOpenSession( TaskSelf() );
		//套接字socket对象,TCP协议
		sockTCPsendVideo = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if( sockTCPsendVideo == INVALID_SOCKET )
		{
		    Roseek_Reset();//如果创建侦听对象失败，重启相机
	    }
		bzero( &connaddr,sizeof(struct sockaddr_in));
		connaddr.sin_family = AF_INET;
		connaddr.sin_addr.s_addr = INADDR_ANY;
		connaddr.sin_len = sizeof( connaddr );
		connaddr.sin_port = htons(0);
		if (bind(sockTCPsendVideo, (PSA) &connaddr,sizeof(connaddr)))
		{
			Roseek_Reset();//如果绑定失败，重启相机
		}	 	
    	bzero( &clientaddr,sizeof(struct sockaddr_in));
		clientaddr.sin_family = AF_INET;
    	clientaddr.sin_len = sizeof( connaddr );
    	clientaddr.sin_port = htons(VIDEOPORT);//端口号
		clientaddr.sin_addr = ((struct sockaddr_in*) &g_ConfigSetaddr)->sin_addr; //指明服务端IP地址
		//连接服务端
		cnt=connect( sockTCPsendVideo, (PSA)&clientaddr, clientaddr.sin_len);
		if(cnt == -1)
		{
             //如果连接出错则关闭接受到的连接对象，重新尝试
			 fdClose(sockTCPsendVideo);
			 continue;
		}
		g_bIsFcontoDecClientConnect = TRUE; //网络线程准备完毕
			
		while(g_bIsSendPictoDec)
		{				
			//发送YUV422原始图像数据至服务端
	    	SEM_pend( &sem_FcontoDecReady, SYS_FOREVER);		    			      	    	           					          
			if( send( sockTCPsendVideo, g_ImgBufofFrontS.pui8FrameBuf, ALG_ROW*ALG_COL*2, 0 ) < 0 )
			{
				break;
			}			  		    
	  	}
	  	//关闭发送服务器连接，重回迭代循环等待新连接		
		fdClose(sockTCPsendVideo);
		g_bIsFcontoDecClientConnect = FALSE;
		SEM_reset( &sem_FcontoDecReady, 0 );
		SEM_reset( &sem_SendVideoReady, 0 );							
	}
}

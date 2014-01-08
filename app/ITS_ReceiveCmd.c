/**
 * \file      	ITS_ReceiveCmd.c
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.1.1
 * \date      	2014.1.8
 * \brief     	智能交通应用——接收红绿灯控制器控制信号的网络线程
 * \            与UDP_Client_CMDMonitor.exe测试通信使用
 * \update		代码整理头文件
**/

#include "EE3_common.h"
#include "EE3_Imgmain.h"
/**************************************************************************/

/**
 * \function 	ReceBpStartSemPort
 * \brief    	使用UDP协议实现控制信号的接收
 * \			默认作为服务器端，Port = 6366	
 * \
**/	
void ReceBpStartSemPort()
{
	SOCKET	sockBpSemListen;
	int		cnt,clilen;
	Bool	bError = FALSE;
	Bool    bSendDp1 = FALSE;
	Bool    bSendDp2 = FALSE;
	struct	sockaddr_in sin; //保存客户的地址信息
	struct  sockaddr pcliaddr;

	//迭代循环
	while( 1 )
	{
		//为当前任务配置运行环境
		fdOpenSession( TaskSelf() );
		//创建侦听套接字socket对象
		sockBpSemListen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//创建服务器监听socket,UDP协议
		if( sockBpSemListen == INVALID_SOCKET )
		{
			Roseek_Reset();//如果创建侦听对象失败，重启相机
		}
		bzero( &sin, sizeof(struct sockaddr_in) );
		sin.sin_family	= AF_INET; 
		sin.sin_addr.s_addr = INADDR_ANY;//自动获取本机地址
		sin.sin_len	= sizeof( sin );
		sin.sin_port	= htons( ACMDPORT );//指明连接服务器的端口号
		//sockBpSemListen绑定
		if ( bind( sockBpSemListen, (PSA) &sin, sizeof(sin) ) < 0 )
		{
			Roseek_Reset();//如果绑定失败，重启相机
		}
		clilen = sizeof(struct sockaddr);
		bError = FALSE;//建立连接成功
    
		while( !bError )
		{
			//迭代接收数据
			cnt=recvfrom( sockBpSemListen, (char *)(&g_BpInputData), sizeof(g_BpInputData), 0, (PSA)&pcliaddr,&clilen);
			if(cnt < 0)
			{
			   	bError = TRUE;
			    break;
			}
			
			//判断信号类型
			if(g_BpInputData.label1 == 0xCC&&g_BpInputData.label2 == 0xCC){//信号类型匹配
				//第一路
				if(g_BpInputData.cameraN == g_EE3Cur.RoadNum[0])
				{
					g_AmcdSem1.bStart=TRUE;
					switch(g_BpInputData.comm)
					{
					   case NETINFORTG://通知红转绿
							g_AmcdSem1.comm=NETINFORTG;//获取控制信号
							break;
					   case NETINFOGTR://通知绿转红
							g_AmcdSem1.comm=NETINFOGTR;
							break;
					   case NETREQUEST://请求队长检测
							g_AmcdSem1.comm=NETREQUEST;	
							bSendDp1=TRUE;		     			
							break;
					   default:
					        break;
					}			     	                	
				}
				//第二路
				if(g_BpInputData.cameraN == g_EE3Cur.RoadNum[1])
				{
					g_AmcdSem2.bStart=TRUE;
					switch(g_BpInputData.comm)
					{
					   case NETINFORTG://通知红转绿
							g_AmcdSem2.comm=NETINFORTG;
							break;
					   case NETINFOGTR://通知绿转红
							g_AmcdSem2.comm=NETINFOGTR;
							break;
					   case NETREQUEST://请求队长检测
							g_AmcdSem2.comm=NETREQUEST;	
							bSendDp2=TRUE;		     			
							break;
					   default:
					        break;
					}			     
				}
					
				//发送队长检测结果
				if(bSendDp1)
				{
					SEM_pend( &sem_DpDataReady1, SYS_FOREVER);
			     	if( sendto( sockBpSemListen,(char *)(&g_BpOutputData1),sizeof(g_BpOutputData1), 0,(PSA)&pcliaddr,clilen )<0)
			     	{
			     		bError = TRUE;
			     		break;
					}
					bSendDp1=FALSE;
				}
				if(bSendDp2)
				{
					SEM_pend( &sem_DpDataReady2, SYS_FOREVER);
			     	if( sendto( sockBpSemListen,(char *)(&g_BpOutputData2),sizeof(g_BpOutputData2), 0,(PSA)&pcliaddr,clilen )<0)
			     	{
			     		bError = TRUE;
			     		break;
					}
					bSendDp2=FALSE;
				}
		    }
		}
		//关闭发送服务器连接，重回迭代循环等待新连接
		fdClose(sockBpSemListen);
		SEM_reset( &sem_DpDataReady1, 0 );
		SEM_reset( &sem_DpDataReady1, 0 );
	}//迭代循环
}







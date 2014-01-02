/**
 * \file      	EE3_SendFconData.c
 * \author    	Leo Bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	FconMode的JPEG图像发送线程
**/

#include "EE3_common.h"

/**************************************************************************/
//外部变量
extern RoseekJPGEncBufStruct	g_JpgEncBufStruct;
extern volatile	Bool	g_bIsFconClientConnect;
/**************************************************************************/

/**
 * \function 	SendFconDataPort
 * \brief    	使用TCP/IP协议实现FconMode的JPEG图像发送
 * \			默认作为服务器端，Port = 55000		
 * \
**/	
void SendFconDataPort()
{
	SOCKET	sockFconListen, sockFconSvr;
	int		size,i,cnt;
	Uint16	ui16Reply = 0;
	Bool	bClosed = FALSE;
	struct	sockaddr_in addr;
	Bool	bBreak=FALSE;
	static	Uint8	ui8JpgBufIndex=0;
	Uint8	ui8tmp;

	//为当前任务配置运行环境
	fdOpenSession( TaskSelf() );

	//创建侦听socket对象
	sockFconListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if( sockFconListen == INVALID_SOCKET ){
		Roseek_Reset();//如果创建侦听对象失败，重启相机
	}
	bzero( &addr, sizeof(struct sockaddr_in) );
	addr.sin_family	= AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_len	= sizeof( addr );
	addr.sin_port	= htons( 55000 );
	//sockFconListen绑定
	if ( bind( sockFconListen, (PSA) &addr, sizeof(addr) ) < 0 ){
		Roseek_Reset();//如果绑定失败，重启相机
	}
	//sockFconListen开始监听，同一时刻仅支持一个连接
	if ( listen( sockFconListen, 1) < 0 ){
		Roseek_Reset();//如果侦听失败，重启相机
	}
	//迭代循环
	while( 1 ){
		size = sizeof( addr );
		sockFconSvr = accept( sockFconListen, (PSA)&addr, &size );
		if( sockFconSvr == INVALID_SOCKET){
			//如果接受连接出错则关闭接受到的连接对象，重新尝试
			fdClose(sockFconSvr);
			continue;
		}
		g_bIsFconClientConnect = TRUE; //网络线程准备完毕
		//向上位机发送MJPG图像
		while( !bClosed ){
			//等待信号灯同步
			SEM_pend( &sem_ConDataReady, SYS_FOREVER);
			//判断上传哪个缓冲区内容
			
			bBreak = FALSE;
			if(ui8JpgBufIndex==1){
				ui8tmp=0;
			}
			else{
				ui8tmp=ui8JpgBufIndex+1;
			}
			
			for( i=ui8tmp; i<ui8tmp+2; i++ ){
				if( i>1 ){
					if( g_JpgEncBufStruct.bJpgImgBuf_Lock[i-2] ){
						ui8JpgBufIndex = i-2;
						bBreak = TRUE;
						break;
					}
				}
				else{//i<=5
					if( g_JpgEncBufStruct.bJpgImgBuf_Lock[i] ){
						ui8JpgBufIndex = i;
						bBreak = TRUE;
						break;
					}
				}
			}
			
			if( !bBreak ){
				continue;
			}

			if( send( sockFconSvr, g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex], *(Uint32*)g_JpgEncBufStruct.pui8JpgImgBuf[ui8JpgBufIndex] + 4, 0 ) < 0 ){
					break;
			}
			g_JpgEncBufStruct.bJpgImgBuf_Lock[ui8JpgBufIndex] = FALSE;

			//接受回应判断是否退出发送循环，若接受到0xffff终止发送循环，否则继续发送数据
			//启动后第一次执行后收到了0xffff(测试时）
			i = 0;
			while( i < 2 ){
				cnt = recv( sockFconSvr, (char*)&ui16Reply, 2-i, 0 );
				if( cnt <= 0 ){
					bClosed = TRUE;
					break;
				}
				i += cnt;
			}
			if( ui16Reply==0xffff ){
				bClosed = TRUE;
			}
		}
		//控制标志复位，关闭发送服务器连接，重回迭代循环等待新连接
		bClosed = FALSE;
		ui16Reply = 0x0000;
		fdClose(sockFconSvr);
		g_bIsFconClientConnect = FALSE;
		g_JpgEncBufStruct.bJpgImgBuf_Lock[0] = FALSE;
		g_JpgEncBufStruct.bJpgImgBuf_Lock[1] = FALSE;
		SEM_reset( &sem_ConDataReady, 0 );
	}//迭代循环
}

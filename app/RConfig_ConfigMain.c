/**
 * \file      	RConfig_ConfigMain.c
 * \author    	L.Q.@Lab217.tongji
 * \version   	0.3.3
 * \date      	2014.1.8
 * \brief     	远程配置功能——远程配置通信的网络线程
 * \update      
**/

#include "EE3_common.h"

/**************************************************************************/
//外部变量
extern volatile    Bool    g_bIsSendPictoDec;
extern EE3_CfgStruct g_EE3Cur;//相机运行状态
extern EE3_StateStruct g_EE3State;
extern unsigned int	g_IPAddrCurUse;//相机IP
/**************************************************************************/
static void Read_DataEncoding(EE3_CfgStruct Cfg, EE3_StateStruct state, char *Sendbuf);
static void Write_ConfigData(Uint8 *ee3cfgBuffer);
/**************************************************************************/

/**
 * \function 	ReceDecOrderPort
 * \brief    	使用TCP/IP协议，远程配置工具通信，配置信息的接收、解析与参数传回
 * \			默认作为服务器端，Port = 4500
 * \
**/	
void ReceDecOrderSemPort()
{
	SOCKET	sockReceDecOrderListen, sockReceDecOrderSvr;
	struct	sockaddr_in addr; //保存客户的地址信息
	int		size,cnt;
	Uint8 	pointBuffer[80];
	char    sendEE3cfgbuf[150];
	Uint8   ee3cfgBuffer[100];		
	Bool	bIsError = FALSE;
	ConversationHead reply;
	ConversationHead g_DecCmdsem;

	//为当前任务配置运行环境
	fdOpenSession( TaskSelf() );
	//创建侦听套接字socket对象
	sockReceDecOrderListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//创建服务器监听socket,TCP协议
	if( sockReceDecOrderListen == INVALID_SOCKET )
	{
		Roseek_Reset();//如果创建侦听对象失败，重启相机
	}
	bzero( &addr, sizeof(struct sockaddr_in) );
	addr.sin_family	= AF_INET; //AF_INET指明使用TCP/IP协议族
	addr.sin_addr.s_addr = INADDR_ANY;//自动获取本机地址
	addr.sin_len = sizeof( addr );
	addr.sin_port = htons( CONTROLPORT );//指明连接服务器的端口号
	//Listen绑定
	if ( bind( sockReceDecOrderListen, (PSA) &addr, sizeof(addr) ) < 0 )
	{
		Roseek_Reset();//如果绑定失败，重启相机
	}
	//开始监听，同一时刻仅支持一个连接
	if ( listen( sockReceDecOrderListen, 1) < 0 )
	{
		Roseek_Reset();//如果侦听失败，重启相机
	}

	//迭代接受连接
	while(1)
	{
		size = sizeof( addr );
		sockReceDecOrderSvr = accept( sockReceDecOrderListen, (PSA) &addr, &size );
		if( sockReceDecOrderSvr == INVALID_SOCKET)
		{
			//如果接受连接出错则关闭接受到的连接对象，重新尝试
			fdClose(sockReceDecOrderSvr);
			continue;
		}
		bIsError = FALSE;//连接成功标记
		
		//迭代接受命令
		while(!bIsError)
		{	
			cnt=recv( sockReceDecOrderSvr, (char *)(&g_DecCmdsem), sizeof(g_DecCmdsem), 0 );
			if(cnt<0)
			{
			  	bIsError = TRUE;
			  	break;
			}
			
			//配置功能命令解析
			switch(g_DecCmdsem.command)
			{
				  //1选择相机
			      case CHOOSECAM:
					 //反馈信息
				     reply.command = CHOOSECAM;
					 reply.bufsize = g_DecCmdsem.bufsize;					 
                     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
                     {
						bIsError = TRUE;
                     	break;
					 }
					 break;

				  //2请求发送图像
				  case STARTVIDEO:
					 //反馈信息
				     reply.command = STARTVIDEO;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 SEM_post(&sem_SendVideoReady);//传输图像线程开启信号
				     g_bIsSendPictoDec = TRUE; 
					 break;

				  //3终止发送图像
				  case ENDVIDEO:
				  	 //反馈信息				     
				     reply.command = ENDVIDEO;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 g_bIsSendPictoDec = FALSE;//停止传输图像
					 break;

				  //4虚拟线圈配置
				  case SETPOINT:
					 //接收配置数据
				     if(recv( sockReceDecOrderSvr, (char *)(&pointBuffer), sizeof(pointBuffer), 0 )<0)
				     {
				     	bIsError = TRUE;
				     	break;
					 }
					 //数据存入FLASH
					 switch(g_EE3Cur.RunMode)
					 {
						case VEHICLE_DETECT:
					 		Roseek_Flash_Burn(pointBuffer,CFGVDPOINTADD,80); 
							break;
						case QUEUE_DETECT:
					 		Roseek_Flash_Burn(pointBuffer,CFGQDPOINTADD,80);	
					 		break;
					 	default:
					 		break;
					 }
					 //反馈信息
					 reply.command = SETPOINT;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }																										
					 break;

				  //5发送相机状态信息
				  case EE3_SENDCONFIG:
					 //反馈信息
					 reply.command = EE3_SENDCONFIG;
					 reply.bufsize = sizeof(sendEE3cfgbuf);//返回发送包的大小
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 //打包数据,准备发送
					 Read_DataEncoding(g_EE3Cur, g_EE3State, sendEE3cfgbuf);
					 //发送配置信息
				     if(send( sockReceDecOrderSvr, &sendEE3cfgbuf, sizeof(sendEE3cfgbuf), 0 )<0)
				     {
				     	bIsError = TRUE;
					 	break;
					 }
					 break;

				  //6接收配置信息
				  case EE3_SETCONFIG:
					 //接收配置数据
				     if(recv( sockReceDecOrderSvr, (char *)(&ee3cfgBuffer), sizeof(ee3cfgBuffer), 0 )<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 //写入配置数据
					 Write_ConfigData(ee3cfgBuffer);
					 //反馈信息
					 reply.command = EE3_SETCONFIG;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 break;

				  //7重启相机
				  case REBOOT:
					 //反馈信息
					 reply.command = REBOOT;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
				     Roseek_Reset();
					 break;

				  //8取消当前操作
				  case CANCELCMD:
					 //反馈信息
					 reply.command = CANCELCMD;
					 reply.bufsize = 1;
				     if(send(sockReceDecOrderSvr,&reply,sizeof(reply),0)<0)
				     {
						bIsError = TRUE;
                     	break;
					 }
					 //配置完成结束本次连接（new）
					 bIsError = TRUE;				     
					 break;

			      default:
	                 break;
			}
			

		}//while recv
				
		//关闭连接，回迭代循环
		fdClose(sockReceDecOrderSvr);
	}//迭代循环
	
}

/**
 * \function 	StateDataPackup读取相机当前配置信息，打包格式封装
 * \brief    	输入：Cfg、state当前相机运行参数
 * \			输出：*Sendbuf打包后的字符流
 * \			注意：1、参数行顺序不可调整，只能逐行添加。
 * \
**/
static void Read_DataEncoding(EE3_CfgStruct Cfg, EE3_StateStruct state, char *Sendbuf)
{
	char runmode[5];
	char cameraip[15];
	char resolution[5];
	char temperature[5];
	char serverip[15];
	char serverport[7];
	char roadnum[10];
	char sroad1[5];
	char sroad2[5];
	char uploadtime[5];
	char nighttrafficstream[5];
	char version[40];
	char isreadflash[5];
	float   ftmpnow;
	unsigned int serverIp;
		
	//请按顺序打包封装
	//第1行，RunMode
	sprintf(runmode,"%d",(int)(Cfg.RunMode));
	strcat(runmode,"\r\n");
	//第2行，CamraIp,相机IP
	sprintf(cameraip,"%d",g_IPAddrCurUse);
	strcat(cameraip,"\r\n");
	//第3行，Resolution,相机分辨率
	sprintf(resolution,"%d",(int)(state.Resolution));
	strcat(resolution,"\r\n");
	//第4行，Temperature，相机温度
	ftmpnow = Roseek_Read_CurTemperature(); 	
	Roseek_Waitusec( 10 );				 					
	sprintf(temperature,"%d",(int)(ftmpnow));
	strcat(temperature,"\r\n");
	//第5行，ServerIp
	serverIp = inet_addr(Cfg.ServerIp);
	sprintf(serverip,"%d",serverIp);
	strcat(serverip,"\r\n");
	//第6行，ServerPort
	sprintf(serverport,"%d",(int)(Cfg.ServerPort));
	strcat(serverport,"\r\n");
	//第7行，RoadNum
	sprintf(sroad1,"%d",Cfg.RoadNum[0]);
	sprintf(sroad2,"%d",Cfg.RoadNum[1]);
	strcat(sroad1,"\r\n");
	strcat(sroad2,"\r\n");
	strcpy((char *)(&roadnum),sroad1);
	strcat((char *)(&roadnum),sroad2);
	//第8行，UploadTime
	sprintf(uploadtime,"%d",(int)(Cfg.UploadTime));
	strcat(uploadtime,"\r\n");
	//第9行，NightTrafficStream
	sprintf(nighttrafficstream,"%d",(int)(Cfg.NightTrafficStream));
	strcat(nighttrafficstream,"\r\n");
	//第10行，Version
	strcpy(version,state.Version);
	strcat(version,"\r\n");
	//第11行，Isreadflash
	sprintf(isreadflash,"%d",(int)(Cfg.Isreadflash));
	strcat(isreadflash,"\r\n");
	//此处继续添加
	
	//信息封包
	strcpy((char *)(Sendbuf),runmode);//1
	strcat((char *)(Sendbuf),cameraip);//2
	strcat((char *)(Sendbuf),resolution);//3
	strcat((char *)(Sendbuf),temperature);//4
	strcat((char *)(Sendbuf),serverip);//5
	strcat((char *)(Sendbuf),serverport);//6
	strcat((char *)(Sendbuf),roadnum);//7
	strcat((char *)(Sendbuf),uploadtime);//8
	strcat((char *)(Sendbuf),nighttrafficstream);//9
	strcat((char *)(Sendbuf),version);//10
	strcat((char *)(Sendbuf),isreadflash);//11
	//此处继续添加
	
}

/**
 * \function 	Write_ConfigData读取配置信息，写入FLASH
 * \brief    	输入：ee3cfgBuffer接受到的配置信息
 * \			解析出IP地址，并修改IP
 * \			注意：
 * \update		13.12.31修正BUG，防止错误的IP地址存入
**/
static void Write_ConfigData(Uint8 *ee3cfgBuffer)
{
	int i, j;
	static  int     ee3_mode;
	static  Bool  bSchange = FALSE;
	static  Bool  bIsIP = FALSE;
	char    scommonmode[80];
	char    sfmode[30];
	char    StringIP[30];
	unsigned int  tempIP;

	//读取运行功能
	ee3_mode = (int)(ee3cfgBuffer[0] - '0');//读取功能模式
	//配置信息转存
	for(i = 0,j = 0; ;i++)
	{
		if(ee3cfgBuffer[i] == '\v')
			break;
		if(ee3cfgBuffer[i] == '\f')
		{
			bSchange = TRUE;
			continue;							
		}
		if(!bSchange)
			scommonmode[i] = ee3cfgBuffer[i];
		else
		{
			sfmode[j] = ee3cfgBuffer[i];
			j++;
		}						
	}

	//读IP地址并修改
	for(i = 2;i < 30;i++)
	{
		//'\r'为结束符
		if(ee3cfgBuffer[i]!='\r')
		{
			//判断是否为IP
			if(('0' <= ee3cfgBuffer[i] && ee3cfgBuffer[i] <= '9' )|| ee3cfgBuffer[i] == '.')
			{
				StringIP[i-2]=ee3cfgBuffer[i];
				bIsIP = TRUE;
			}
			else
			{
				bIsIP = FALSE;
			}
		}
		else
		{
			StringIP[i-2]='\0';//添加字符串尾
			break;
		}
	}
	if(bIsIP == TRUE)
	{
		tempIP = inet_addr(StringIP);
		//修改相机的IP地址
		Roseek_Save_IP_Address( tempIP );
	}
	//写入Flash
	Roseek_Flash_Burn((Uint8 *)scommonmode,CFGCOMMONADD,80);//存储通用配置信息
	switch(ee3_mode)
	{
		case VEHICLE_DETECT:
			Roseek_Flash_Burn((Uint8 *)sfmode,CFGVDADD,30);
			break;
		case QUEUE_DETECT:
			Roseek_Flash_Burn((Uint8 *)sfmode,CFGQDADD,30);
			break;
		case TEST_MODE:
			break;
		default:
			break;
	}
	
}





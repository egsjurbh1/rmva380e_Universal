#ifndef _SOCK_H_
#define _SOCK_H_

#include "RtspCommon.h"

#ifdef WIN32
#include <WinSock2.h>
#define MSG_NOSIGNAL 0
#define SOCKET_TYPE	int
#else
#ifdef PLATFORM_DSP
#include <netmain.h>
#define MSG_NOSIGNAL 0
#define select fdSelect
#define SOCKET_TYPE	SOCKET
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#define SOCKET_TYPE	int
#endif
#endif

SOCKET_TYPE	sock_createTcp();
SOCKET_TYPE	sock_createUdp();
void sock_close(SOCKET_TYPE sock);
int sock_bind(SOCKET_TYPE sock, const char *pIP, unsigned short port);
int sock_listen(SOCKET_TYPE sock, int num);
SOCKET_TYPE sock_accept(SOCKET_TYPE sock);
int sock_connect(SOCKET_TYPE sock, const char *pIP, unsigned short port, int timeout);
int sock_send(SOCKET_TYPE sock, const char *pBuf, int len);
int sock_recv(SOCKET_TYPE sock, char *pBuf, int len, int flag);
int sock_safeSend(SOCKET_TYPE sock, const char *pBuf, int len);
int sock_safeRecv(SOCKET_TYPE sock, char *pBuf, int len, int flag);
int sock_sendto(SOCKET_TYPE sock, const char *pBuf, int len, const struct sockaddr_in *pAddr);
int sock_recvfrom(SOCKET_TYPE sock, char *pBuf, int len, struct sockaddr_in *pAddr);
int sock_isCanRead(SOCKET_TYPE sock, int timeout);
int sock_isCanWrite(SOCKET_TYPE sock, int timeout);
int sock_getAddr(SOCKET_TYPE sock, struct sockaddr_in *pAddr);
int sock_getPeerAddr(SOCKET_TYPE sock, struct sockaddr_in *pAddr);
int sock_getHostAddr(struct sockaddr_in *pAddr);
int sock_setBlocking(SOCKET_TYPE sock, int isBlocking);

#endif




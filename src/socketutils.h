#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H

#include "protocolutils.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <mswsock.h>  
    #include <windows.h>

    #include <ws2tcpip.h>

    int inet_pton(
        int af,
        const char *src,
        void *dst);
    const char *inet_ntop(
        int af,
        const void *src,
        char *dst,
        socklen_t size);
#endif

#define GOBIO_BUILD_VERSION "1.0.0-beta"
#define MAX_BUFF_SIZE 384
#define DEFAULT_PORTN 7070

request_t *(*g_ParseRequest)(char *);
table_t *g_RouteTable;
char *(*g_ExecuteRoute)(request_t *, table_t *);
char *(*g_DefaultRoute)(request_t *);

int g_ThreadCount;

HANDLE g_hIOCP = INVALID_HANDLE_VALUE;  
SOCKET g_ServerSocket = INVALID_SOCKET;  

typedef enum {
    IO_READ,
    IO_WRITE,
    IO_CLOSE
} IO_OPERATION;  

typedef struct IO_DATA {  
    WSAOVERLAPPED   Overlapped;
    char            Buffer[MAX_BUFF_SIZE];
    WSABUF          WSABuf;  
    int             nTotalBytes;  
    int             nSentBytes;  
    IO_OPERATION    opCode;  
    SOCKET          activeSocket;  
} IO_DATA;  

int initservice (
    request_t *(*ParseRequest)(char *),
    table_t *RouteTable,
    char *(*ExecuteRoute)(request_t *, table_t *),
    char *(*DefaultRoute)(request_t *));

#endif
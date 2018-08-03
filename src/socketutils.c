#include "protocolutils.h"
#include "socketutils.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <mswsock.h>  
    #include <windows.h>

    int inet_pton(int af, const char *src, void *dst) {
        struct sockaddr_storage ss;
        int size = sizeof(ss);
        char src_copy[INET6_ADDRSTRLEN + 1];

        ZeroMemory(&ss, sizeof(ss));
        // Non-const API unfortunately
        strncpy (src_copy, src, INET6_ADDRSTRLEN + 1);
        src_copy[INET6_ADDRSTRLEN] = 0;

        if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
            switch(af) {
            case AF_INET:
            *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
            return 1;
            case AF_INET6:
            *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
            return 1;
            }
        }
        return 0;
    }

    const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
        struct sockaddr_storage ss;
        unsigned long s = size;

        ZeroMemory(&ss, sizeof(ss));
        ss.ss_family = af;

        switch(af) {
            case AF_INET:
            ((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
            break;
            case AF_INET6:
            ((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
            break;
            default:
            return NULL;
        }
        /* cannot direclty use &size because of strict aliasing rules */
        return (WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0)?
                dst : NULL;
    }
#endif



// Need to link with Ws2_32.lib
// #pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
// gcc <source>.c -o <executable> -lwsock32 -lws2_32

DWORD WINAPI WorkerThread (LPVOID WorkThreadContext) {
    LPWSAOVERLAPPED lpOverlapped = NULL;
    IO_DATA *lpIOContext = NULL;

    DWORD dwRecvNumBytes = 0;
    DWORD dwSendNumBytes = 0;
    DWORD dwFlags = 0;
    DWORD dwIoSize = 0;
    BOOL bSuccess = FALSE;

    int nRet = 0;

        while (1)
        {
            void *lpCompletionKey = NULL;  
            bSuccess = GetQueuedCompletionStatus(g_hIOCP,
                                                &dwIoSize,  
                                                (PULONG_PTR)&lpCompletionKey,  
                                                (LPOVERLAPPED *)&lpOverlapped,   
                                                INFINITE);  
            if (!bSuccess) {
                printf("GetQueuedCompletionStatus() error diagnosis %s\n", GetLastError());  
                break;  
            }  

            lpIOContext = (IO_DATA *)lpOverlapped;  

            if (lpIOContext->opCode == IO_READ) // A read operation already complete, write next
            {  
                lpIOContext->nTotalBytes  = lpIOContext->WSABuf.len;  
                lpIOContext->nSentBytes   = 0;
                lpIOContext->opCode = IO_CLOSE;  
                dwFlags = 0;

                request_t *ParsedRequest = g_ParseRequest(lpIOContext->Buffer);

                char *Buffer = NULL;
                Buffer = g_ExecuteRoute(ParsedRequest, g_RouteTable);

                if (Buffer == NULL)
                    Buffer = g_DefaultRoute(ParsedRequest);
                ZeroMemory(lpIOContext->Buffer, sizeof(lpIOContext->Buffer));
                CopyMemory(lpIOContext->Buffer, Buffer, sizeof(lpIOContext->Buffer));

                nRet = WSASend(  
                            lpIOContext->activeSocket,
                            &lpIOContext->WSABuf,
                            1,
                            &dwSendNumBytes,  
                            dwFlags,  
                            &(lpIOContext->Overlapped),
                            NULL);

                free_(Buffer); // Better be sure of it!

                if (nRet == SOCKET_ERROR && (ERROR_IO_PENDING != WSAGetLastError())) {
                        printf("WASSend() error diagnosis %s\n", WSAGetLastError());  
                        closesocket(lpIOContext->activeSocket);  
                        free(lpIOContext);  
                        continue;  
                }  
        } else if (lpIOContext->opCode == IO_CLOSE)  {
            // printf("Connection ended\n");
			// Clean up all the WSASend/WSARecv communication and makes the engine ready for a new one
			if (shutdown(lpIOContext->activeSocket, SD_BOTH) != 0)
			    printf("\nshutdown() error diagnosis %s\n", WSAGetLastError());

            closesocket(lpIOContext->activeSocket);  
            free(lpIOContext);  
            continue;  

        } 
    }  
    return 0;  
}

int initservice (
                request_t *(*ParseRequest)(char *),
                table_t *RouteTable,
                char *(*ExecuteRoute)(request_t *, table_t *),
                char *(*DefaultRoute)(request_t *) ){
    g_ParseRequest = ParseRequest;
    g_RouteTable = RouteTable;
    g_ExecuteRoute = ExecuteRoute;
    g_DefaultRoute = DefaultRoute;
    
    {   // Initialize winsock2 library  
        WSADATA WSAData;  
        ZeroMemory(&WSAData, sizeof(WSADATA));  
        int retVal = -1;  
        if ((retVal = WSAStartup(MAKEWORD(2, 2), &WSAData)) != 0) {  
            printf("WSAStartup() error code %s\n", retVal);
        }  
    }  
    {   // Create WSASocket instance
        g_ServerSocket = WSASocket(
                                    AF_INET,
                                    SOCK_STREAM,
                                    IPPROTO_TCP,
                                    NULL,
                                    0,
                                    WSA_FLAG_OVERLAPPED);  
        if (g_ServerSocket == INVALID_SOCKET) {  
            printf("WSASocket() error diagnosis %s\n", WSAGetLastError());
        }
    }  
    {   // Binding
        struct sockaddr_in service;  
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = htonl(INADDR_ANY);
        service.sin_port = htons(INPORT_DEF);
        int retVal = bind(
                        g_ServerSocket,
                        (SOCKADDR *)&service,
                        sizeof(service));
        if (retVal == SOCKET_ERROR) {  
            printf("bind() error diagnosis %s\n", WSAGetLastError());    
        }  
    }  
    {   // Listening
        int retVal = listen(g_ServerSocket, 8);  
        if (retVal == SOCKET_ERROR) {
            printf("listen() error diagnosis %s\n", WSAGetLastError());
        }  
    }  
    {   // Create IOCP  
        SYSTEM_INFO sysInfo;  
        ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));  
        GetSystemInfo(&sysInfo);  
        g_ThreadCount = sysInfo.dwNumberOfProcessors * 2;  
        g_hIOCP = CreateIoCompletionPort(
                                        INVALID_HANDLE_VALUE,
                                        NULL,
                                        0,
                                        g_ThreadCount);  
        if (g_hIOCP == NULL) {  
            printf("CreateIoCompletionPort() error diagnosis %s\n", GetLastError());    
        }  
        if (CreateIoCompletionPort((HANDLE)g_ServerSocket, g_hIOCP, 0, 0) == NULL) {  
            printf("CreateIoCompletionPort() binding error diagnosis %s\n", GetLastError()); 
        }  
    }  
    {  // Create worker threads  
        for (DWORD dwThread = 0; dwThread < g_ThreadCount; dwThread++)  
        {  
            HANDLE  hThread;  
            DWORD   dwThreadId;  
            hThread = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                0,
                                0,
                                &dwThreadId);  
            // CloseHandle(hThread); 
        }  
    }  
    { // Accepting new connections  
        while(1)  
        {  
            SOCKET ls = accept(
                            g_ServerSocket,
                            NULL,
                            NULL);  
            if (ls == SOCKET_ERROR) break;
            // printf("Connection accepted\n");
            { // Disables buffer to improve performance  
                int nZero = 0;  
                setsockopt(
                        ls,
                        SOL_SOCKET,
                        SO_SNDBUF,
                        (char *)&nZero,
                        sizeof(nZero));
            }
            if (CreateIoCompletionPort((HANDLE)ls, g_hIOCP, 0, 0) == NULL) {
                printf("CreateIoCompletionPort() binding error diagnosis %s\n", GetLastError());  
                closesocket(ls);
            }
            else { // Issues a WSARecv request  
                IO_DATA *data = (IO_DATA *)malloc(sizeof(struct IO_DATA));  
                ZeroMemory(&data->Overlapped, sizeof(data->Overlapped));
                ZeroMemory(data->Buffer, sizeof(data->Buffer));
                data->opCode       = IO_READ;  
                data->nTotalBytes  = 0;  
                data->nSentBytes   = 0;
                data->WSABuf.buf   = data->Buffer;
                data->WSABuf.len   = sizeof(data->Buffer);  
                data->activeSocket = ls;  
                DWORD dwRecvNumBytes = 0, dwFlags = 0;  
                int nRet = WSARecv(
                                ls,
                                &data->WSABuf,
                                1,
                                &dwRecvNumBytes,  
                                &dwFlags,  
                                &data->Overlapped,
                                NULL);
                if (nRet == SOCKET_ERROR  && (ERROR_IO_PENDING != WSAGetLastError())) {  
                    printf("WASRecv() error diagnosis %s\n", WSAGetLastError());  
                    closesocket(ls);  
                    free(data);  
                }  
            }  
        }  
    }  
    closesocket(g_ServerSocket);  
    WSACleanup();  
    return 0;
}
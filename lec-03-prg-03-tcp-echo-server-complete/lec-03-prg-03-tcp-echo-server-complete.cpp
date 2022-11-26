#include <iostream>
#include <stdio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

#define PORT 65456
#define PACKET_SIZE 1024

int main()
{

    std::cout << "> echo-server is activated" << std::endl;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "error\n";
        return 0;
    }

    SOCKET hListen;
    hListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN tListenAddr = {};
    tListenAddr.sin_family = AF_INET;
    tListenAddr.sin_port = htons(PORT);
    tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr)) == SOCKET_ERROR) {
        __int32 errorCode = WSAGetLastError();
        std::cout << "> bind() failed and program terminated. error code : " << errorCode << std::endl;
        closesocket(hListen);
        return 0;
    }

    if (listen(hListen, SOMAXCONN) == SOCKET_ERROR) {
        __int32 errorCode = WSAGetLastError();
        std::cout << "> listen() failed and program terminated. error code : " << errorCode << std::endl;
        closesocket(hListen);
        return 0;
    }

    SOCKADDR_IN tClntAddr = {};
    int iClntSize = sizeof(tClntAddr);
    SOCKET hClient = accept(hListen, (SOCKADDR*)&tClntAddr, &iClntSize);
    std::cout << "> client connected by IP address " << tClntAddr.sin_addr.s_addr << " with Port number " << tClntAddr.sin_port << std::endl;
    while (true) {
        char cBuffer[PACKET_SIZE] = {};
        recv(hClient, cBuffer, PACKET_SIZE, 0);
        std::cout << "> echoed: " << cBuffer << std::endl;
        send(hClient, cBuffer, strlen(cBuffer), 0);
        if (strcmp(cBuffer, "quit") == 0)
            break;
    }

    closesocket(hClient);
    closesocket(hListen);

    WSACleanup();

    std::cout << "> echo-server is de-activated" << std::endl;

}

#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32")

#define PORT 65456
#define PACKET_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main()
{
    std::cout << "> echo-client is activated\n";

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET hSocket;
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN tAddr = {};
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = htons(PORT);

    //tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    inet_pton(AF_INET, SERVER_IP, &(tAddr.sin_addr));

    connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr));

    while (true) {
        char sendMsg[PACKET_SIZE] = {};
        std::cout << "> ";
        std::cin >> sendMsg;
        send(hSocket, sendMsg, strlen(sendMsg), 0);

        char recvData[PACKET_SIZE] = {};
        recv(hSocket, recvData, PACKET_SIZE, 0);
        std::cout << "> received: " << recvData << std::endl;
        if (strcmp(recvData, "quit") == 0)
            break;
    }
    closesocket(hSocket);

    std::cout << "> echo-client is de-activated" << std::endl;

    WSACleanup();
}

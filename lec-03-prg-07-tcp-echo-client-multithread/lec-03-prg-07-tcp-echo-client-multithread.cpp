#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32")

#define PORT 65456
#define PACKET_SIZE 1024
#define SERVER_IP "127.0.0.1"
bool isContinue;

int clientHandler(SOCKET hSocket) {
    while (true) {
        char recvData[PACKET_SIZE] = {};
        recv(hSocket, recvData, PACKET_SIZE, 0);
        std::cout << "> received: " << recvData << std::endl << "> ";
        if (strcmp(recvData, "quit") == 0 && isContinue == false)
            break;
    }
    return 0;
}

int main()
{
    std::cout << "> echo-client is activated\n";
    isContinue = true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "error\n";
        return 0;
    }

    SOCKET hSocket;
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    SOCKADDR_IN tAddr = {};
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = htons(PORT);

    //tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    inet_pton(AF_INET, SERVER_IP, &(tAddr.sin_addr));

    if (connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr)) == SOCKET_ERROR) {
        __int32 errorCode = WSAGetLastError();
        std::cout << "> connect() failed and program terminated. error code : " << errorCode << std::endl;
        closesocket(hSocket);
        return 0;
    }

    std::thread mainThread(clientHandler, hSocket);
    mainThread.detach();

    while (true) {
        char sendMsg[PACKET_SIZE] = {};
        std::cin >> sendMsg;
        send(hSocket, sendMsg, strlen(sendMsg), 0);
        if (strcmp(sendMsg, "quit") == 0) {
            isContinue = false;
            break;
        }
    }
    closesocket(hSocket);

    std::cout << "> echo-client is de-activated" << std::endl;

    WSACleanup();
}
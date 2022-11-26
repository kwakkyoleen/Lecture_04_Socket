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
    SOCKADDR_IN tAddr;

    int tSize = sizeof(tAddr);
    while (true) {
        char recvData[PACKET_SIZE] = {};
        if (recvfrom(hSocket, recvData, PACKET_SIZE, 0, (SOCKADDR*)&tAddr, &tSize) > 0) {
            std::cout << "> received: " << recvData << std::endl << "> ";
            if (strcmp(recvData, "quit") == 0 && isContinue == false)
                break;
        }
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
    hSocket = socket(AF_INET, SOCK_DGRAM, 0);

    SOCKADDR_IN tAddr = {};
    tAddr.sin_family = AF_INET;
    tAddr.sin_port = htons(PORT);

    //tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    inet_pton(AF_INET, SERVER_IP, &(tAddr.sin_addr));

    std::thread mainThread(clientHandler, hSocket);
    mainThread.detach();

    while (true) {
        char sendMsg[PACKET_SIZE] = {};
        std::cout << "> ";
        std::cin >> sendMsg;
        sendto(hSocket, sendMsg, strlen(sendMsg), 0, (SOCKADDR*)&tAddr, sizeof(tAddr));
        if (strcmp(sendMsg, "quit") == 0) {
            isContinue = false;
            break;
        }
    }
    closesocket(hSocket);

    std::cout << "> echo-client is de-activated" << std::endl;

    WSACleanup();
}
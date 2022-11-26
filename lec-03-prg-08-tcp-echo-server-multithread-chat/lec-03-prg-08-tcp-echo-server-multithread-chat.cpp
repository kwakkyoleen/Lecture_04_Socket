#include <iostream>
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32")

#define PORT 65456
#define PACKET_SIZE 1024

int totalThreadNum;
std::mutex threadNumMutex;

class MyTCPSocketHandler {
private:
    int totalThreadNum;
    std::mutex threadNumMutex;
    int PortNum;
    std::string HostIp;
    std::vector<SOCKET> group_queue;
    std::mutex group_queue_Mutex;

    int sendMsgHandler(char* cBuffer) {
        group_queue_Mutex.lock();
        for (std::vector<SOCKET>::iterator iter = group_queue.begin(); iter != group_queue.end(); iter++) {
            send(*iter, cBuffer, strlen(cBuffer), 0);
        }
        group_queue_Mutex.unlock();
        return 0;
    }

    int tcpSocketHandler(SOCKET hClient) {
        bool whileCondition = true;
        while (whileCondition) {
            char cBuffer[PACKET_SIZE] = {};
            recv(hClient, cBuffer, PACKET_SIZE, 0);
            std::cout << "> echoed: " << cBuffer << " by " << std::this_thread::get_id() << std::endl;
            std::stringstream ss;
            ss << " by " << std::this_thread::get_id();
            char tempBuffer[PACKET_SIZE];
            strcpy_s(tempBuffer, sizeof(tempBuffer), cBuffer);
            strcat_s(tempBuffer, sizeof(tempBuffer), ss.str().c_str());
            if (strcmp(cBuffer, "quit") == 0) {
                group_queue_Mutex.lock();
                for (std::vector<SOCKET>::iterator iter = group_queue.begin(); iter != group_queue.end();) {
                    if (*iter == hClient) {
                        group_queue.erase(iter);
                        break;
                    }
                    else {
                        iter++;
                    }
                }
                group_queue_Mutex.unlock();
                whileCondition = false;
            }
            std::thread sendMsg([&]() { MyTCPSocketHandler::sendMsgHandler(tempBuffer); });
            sendMsg.join();
        }
        closesocket(hClient);
        threadNumMutex.lock();
        totalThreadNum--;
        threadNumMutex.unlock();
        return 0;
    }

    int tcpAcceptHandler(SOCKET hListen) {
        while (true) {
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
            group_queue_Mutex.lock();
            group_queue.push_back(hClient);
            group_queue_Mutex.unlock();
            std::thread tempThread([&]() {MyTCPSocketHandler::tcpSocketHandler(hClient); });
            tempThread.detach();
            threadNumMutex.lock();
            totalThreadNum++;
            threadNumMutex.unlock();
        }

        return 0;
    }

public:
    MyTCPSocketHandler(std::string _HostIp, int _PortNum) {
        totalThreadNum = 0;
        HostIp = _HostIp;
        PortNum = _PortNum;
    }

    ~MyTCPSocketHandler() {

    }

    int serve_forever() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "error\n";
            return -1;
        }

        SOCKET hListen;
        hListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        SOCKADDR_IN tListenAddr = {};
        tListenAddr.sin_family = AF_INET;
        tListenAddr.sin_port = htons(PortNum);
        if (HostIp == "localhost") {
            tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else {
            inet_pton(AF_INET, HostIp.c_str(), &(tListenAddr.sin_addr));
        }
        if (bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr)) == SOCKET_ERROR) {
            __int32 errorCode = WSAGetLastError();
            std::cout << "> bind() failed and program terminated. error code : " << errorCode << std::endl;
            closesocket(hListen);
            return -1;
        }

        std::thread mainThread([&]() {MyTCPSocketHandler::tcpAcceptHandler(hListen); });
        std::cout << "> server loop running in thread (main thread): " << mainThread.get_id() << std::endl;

        mainThread.detach();
        while (true) {
            char msg[PACKET_SIZE] = {};
            std::cout << "> ";
            std::cin >> msg;
            if (strcmp(msg, "quit") == 0) {
                if (totalThreadNum <= 0) {
                    std::cout << "> stop procedure started\n";
                    break;
                }
                else {
                    std::cout << "> active threads are remained : " << totalThreadNum << "threads\n";
                }
            }
        }

        closesocket(hListen);

        WSACleanup();
    }
};

int main()
{

    std::cout << "> echo-server is activated" << std::endl;

    MyTCPSocketHandler server("localhost", PORT);
    server.serve_forever();

    std::cout << "> echo-server is de-activated" << std::endl;
    Sleep(100);
}
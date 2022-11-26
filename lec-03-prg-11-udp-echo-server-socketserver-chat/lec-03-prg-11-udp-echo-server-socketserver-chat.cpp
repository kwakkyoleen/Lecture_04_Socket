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

class MyUDPSocketHandler {
private:
    int totalThreadNum;
    std::mutex threadNumMutex;
    int PortNum;
    std::string HostIp;
    std::vector<SOCKADDR_IN> group_queue;
    std::mutex group_queue_Mutex;

    int sendMsgHandler(SOCKET hListen, char* cBuffer) {
        group_queue_Mutex.lock();
        for (std::vector<SOCKADDR_IN>::iterator iter = group_queue.begin(); iter != group_queue.end(); iter++) {
            sendto(hListen, cBuffer, PACKET_SIZE, 0, (SOCKADDR*)&(*iter), sizeof(*iter));
        }
        group_queue_Mutex.unlock();
        return 0;
    }

    int udpSocketHandler(SOCKET hListen) {
        bool whileCondition = true;
        SOCKADDR_IN uClientInfo;
        int uClientSize = sizeof(uClientInfo);
        while (whileCondition) {
            char cBuffer[PACKET_SIZE] = {};
            if (recvfrom(hListen, cBuffer, PACKET_SIZE, 0, (SOCKADDR*)&uClientInfo, &uClientSize) > 0) {
                if (cBuffer[0] == '#' || strcmp(cBuffer, "quit") == 0) {
                    if (strcmp(cBuffer, "#REG") == 0) {
                        std::cout << "> client registered: " << uClientInfo.sin_addr.s_addr << std::endl;
                        group_queue_Mutex.lock();
                        group_queue.push_back(uClientInfo);
                        group_queue_Mutex.unlock();
                    }
                    else if (strcmp(cBuffer, "#DEREG") == 0 || strcmp(cBuffer, "quit ") == 0) {
                        std::cout << "> client de-registered: " << uClientInfo.sin_addr.s_addr << std::endl;
                        group_queue_Mutex.lock();
                        for (std::vector<SOCKADDR_IN>::iterator iter = group_queue.begin(); iter != group_queue.end(); iter++) {
                            if (iter->sin_addr.s_addr == uClientInfo.sin_addr.s_addr && iter->sin_port == uClientInfo.sin_port) {
                                group_queue.erase(iter);
                                break;
                            }
                        }
                        group_queue_Mutex.unlock();
                    }
                }
                else {
                    if (group_queue.empty())
                        std::cout << "> no clients to echo " << std::endl;
                    else {
                        bool attacherInQ = false;
                        group_queue_Mutex.lock();
                        for (std::vector<SOCKADDR_IN>::iterator iter = group_queue.begin(); iter != group_queue.end(); iter++) {
                            if (iter->sin_addr.s_addr == uClientInfo.sin_addr.s_addr && iter->sin_port == uClientInfo.sin_port) {
                                attacherInQ = true;
                                break;
                            }
                        }
                        group_queue_Mutex.unlock();
                        if (attacherInQ) {
                            std::cout << "> received ( " << cBuffer << " ) and echoed to " << group_queue.size() << " clients" << std::endl;
                            std::thread sendThread([&]() {sendMsgHandler(hListen, cBuffer); });
                            sendThread.join();
                        }
                        else {
                            std::cout << "> ignores a message from un-registered client" << std::endl;
                        }
                    }
                }
                //sendto(hListen, cBuffer, PACKET_SIZE, 0, (SOCKADDR*)&uClientInfo, sizeof(uClientInfo));
            }
        }
        return 0;
    }


public:
    MyUDPSocketHandler(std::string _HostIp, int _PortNum) {
        totalThreadNum = 0;
        HostIp = _HostIp;
        PortNum = _PortNum;
    }

    ~MyUDPSocketHandler() {

    }

    int serve_forever() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "error\n";
            return -1;
        }

        SOCKET hListen;
        hListen = socket(AF_INET, SOCK_DGRAM, 0);

        SOCKADDR_IN tListenAddr = {};
        tListenAddr.sin_family = AF_INET;
        tListenAddr.sin_port = htons(PortNum);
        if (HostIp == "localhost") {
            tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else {
            tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        if (bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr)) == SOCKET_ERROR) {
            __int32 errorCode = WSAGetLastError();
            std::cout << "> bind() failed and program terminated. error code : " << errorCode << std::endl;
            closesocket(hListen);
            return -1;
        }

        std::thread mainThread([&]() {MyUDPSocketHandler::udpSocketHandler(hListen); });
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

    MyUDPSocketHandler server("localhost", PORT);
    server.serve_forever();

    std::cout << "> echo-server is de-activated" << std::endl;
    Sleep(100);
}
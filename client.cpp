#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <vector>
#include <arpa/inet.h>
#include <thread>
#include "client.h"
#include <vector>
#include <iostream>       
#include <thread>         
#include <mutex>          
#include <string.h>
#include <fstream>
#include <cstdlib>
#include "latency.h"

using namespace std;

vector <int> serverFds;
vector <thread> serverThreads;
mutex mtx;

int handleNewServer(char* serverIP) {
    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; //localhost for debugging
    serverAddress.sin_port = htons( 5678); //randomly picked


    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    mtx.lock();
    serverFds.push_back(serverFd);
    mtx.unlock();

    if(serverFd == 0) {
        perror("could not create clientfd");
        return -1;
    }

    if(inet_pton(AF_INET, serverIP, &serverAddress.sin_addr)<=0)
    {
        perror("Invalid address\n");
        return -1;
    }

    if (connect(serverFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)    {
        printf("\nConnection Failed \n");
        return -1;
    }


    try {
        char bufferWithSize[256];

        recv(serverFd, bufferWithSize, 256, 0);
        char *chk = nullptr;
        chk = strstr(reinterpret_cast<char *>(bufferWithSize), "ltcTest");
        strtok(bufferWithSize, ",");
        char *size = strtok(nullptr, ",");
        int packetSize = atoi(size);
        if(chk) {
            respondLatency(serverFd, packetSize);
            return 0;
        }
        auto *dataPacket = new size_t[packetSize];
        ssize_t totalReceivedInBytes = 0;
        ssize_t temp = 0;
        char *check = nullptr;
        while (true) {
            temp = recv(serverFd, dataPacket, packetSize, 0);
            if (temp > 0) {
                totalReceivedInBytes = totalReceivedInBytes + temp;
                check = nullptr;
                check = strstr(reinterpret_cast<char *>(dataPacket), "fin");
                if(check) {
                    totalReceivedInBytes = totalReceivedInBytes - 4;
                    break;
                }
                check = nullptr;
                check = strstr(reinterpret_cast<char *>(dataPacket), "LTC");
                if(check) {
                    respondLatency(serverFd, packetSize);
                    return 0;
                }
            } else {
                break;
            }
        }
        char recvStr[120];
        sprintf (recvStr, "%zd", totalReceivedInBytes);
        send(serverFd, recvStr, sizeof(recvStr), 0);
    }
    catch (exception& e) {
        cout << e.what() << '\n';
    }


    return 0;
}

int main(int argc, char *argv[]) {
    //std::cout << "Client process booted up" << std::endl;
    int totalIps = argc - 1;

    for(int i = 1; i < argc; i++) {
    //    std::cout << "Client is to connected to ip " << argv[i] << std::endl;
        thread newServerThread (handleNewServer,argv[i]);
        serverThreads.push_back(move(newServerThread));
    }

    //std::cout << "Connected with servers, waiting till start test." << std::endl;


    for(int i = 0; i < serverThreads.size(); i++){
            serverThreads.at(i).join();
    }
    //cout << "All testing is done, cleaning up client" << endl;
    for(int i = 0; i < serverFds.size(); i++) {
        close(serverFds.at(i));
    }
    return 0;
}

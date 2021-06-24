#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include "server.h"
#include <string.h>
#include <condition_variable>
#include "timer.h"
#include "throughput.h"
#include "latency.h"
#include "util.h"

using namespace std;

int acceptedClients = 0;
vector <int> clientFds;
vector <thread> clientThreads;
volatile bool startSignal = false;
volatile int allInit = 0;
mutex resultsLock;
mutex sendLock;
mutex fileThroughput;
int duration = -1;
ssize_t packetSize = -1;
char serverIP[120];
bool throughput = true;

void handleNewClient(int clientFd, vector <unsigned long> *results, char* clientIP) {
    while(packetSize < 0);
    sendSettingsToReceiver(serverIP, clientIP, throughput, packetSize, clientFd);
    sendLock.lock();
    allInit = allInit + 1;
    sendLock.unlock();
    while (!startSignal);

    if(throughput) {
        ssize_t throughputBytesResult = doThroughputTest(duration, packetSize, clientFd, clientIP, serverIP, fileThroughput);
        resultsLock.lock();
        results->push_back(throughputBytesResult);
        resultsLock.unlock();
        free(clientIP);
    }
    else {
        ssize_t latencyBytesResult = doLatencyTest(duration, packetSize, clientFd,  clientIP, serverIP, fileThroughput);
        resultsLock.lock();
        results->push_back(latencyBytesResult);
        resultsLock.unlock();
        free(clientIP);
    }
}

int main(int argc, char *argv[]) {
    char* p;
    if(argc < 3) {
        perror("Invalid amount of arguments");
        return -1;
    }
    int totalIps = strtol(argv[1], &p, 10);
    if(*p != '\0') {
        perror("invalid number expected clients");
        return -1;
    }

    int option = 1;
    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; //localhost for debugging

    serverAddress.sin_port = htons( 5678 ); //randomly picked

    struct sockaddr_in masterNodeAddress{};
    masterNodeAddress.sin_family = AF_INET;
    masterNodeAddress.sin_addr.s_addr = INADDR_ANY; //localhost for debugging
    masterNodeAddress.sin_port = htons( 5555 ); //randomly picked

    //cout << "Server has been given a total of " << totalIps << " expected clients." << endl;

    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockFd == 0) {
        perror("could not create socketfd");
        return -1;
    }
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR,
                   &option, sizeof(option)))    {
        perror("Failed setsockopt");
        return -1;
    }
    if (setsockopt(sockFd, SOL_SOCKET,  SO_SNDTIMEO,  (char *)&timeout, sizeof(timeout)))    {
        perror("Failed setsockopt");
        return -1;
    }
    if(bind(sockFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("failed to bind");
        return -1;
    }
    if (listen(sockFd, totalIps) < 0) //max backlog in queue consists of number of expected ips
    {
        perror("listenening failed");
        return -1;
    }


    int readyFd;
    int newClient;
    int serverAddressLength = sizeof(serverAddress);

    int masterSock = socket(AF_INET, SOCK_STREAM, 0);
    if(masterSock == 0) {
        perror("could not create masterfd");
        return -1;
    }

    if(inet_pton(AF_INET, argv[2], &masterNodeAddress.sin_addr)<=0)
    {
        perror("Invalid address\n");
        return -1;
    }

    if (connect(masterSock, (struct sockaddr *)&masterNodeAddress, sizeof(masterNodeAddress)) < 0)    {
        printf("\nConnection Failed \n");
        return -1;
    }
    //std::cout << "Connected with master node" << std::endl;

    vector <unsigned long> results;
    while(acceptedClients != totalIps) {
        newClient = accept(sockFd, (struct sockaddr*)&serverAddress, (socklen_t*)&serverAddressLength);
        if(newClient < 0) {
            perror("failed accepting client");
            return -1;
        }
        clientFds.push_back(newClient);
        char* clientIP = (char*)malloc(128);
        strcpy(clientIP,(inet_ntoa(serverAddress.sin_addr)));
        thread newClientThread(handleNewClient,newClient, &results, clientIP);
        clientThreads.push_back(move(newClientThread));
        acceptedClients++;
    }

    //cout << "All clients are connected, signaling masternode, waiting till start test" << endl;
    const char *ready = "ready";

    send(masterSock, ready,  5, 0 );

    char masternodeResponse[1024];
    int bytesRead = 0;
    char start[] = "start";

    recv(masterSock, masternodeResponse, 1, 0 );

    while(*(masternodeResponse + bytesRead) != ';') {
        bytesRead = bytesRead + 1;
        recv(masterSock, masternodeResponse+bytesRead, 1, 0 );
    }

    char *chk = nullptr;
    chk = strstr(reinterpret_cast<char *>(masternodeResponse), "ltc");
    if (chk) {
        throughput = false;
    }

    char *check;
    check = strtok(masternodeResponse, ",");

    if(check) {
        //cout << "found start signal" << endl;
        char *time = strtok(nullptr, ",");
        char *size = strtok(nullptr, ",");
        char *serverIp = strtok(nullptr, ",");
        strcpy(serverIP, serverIp);
        duration = atoi(time);
        packetSize = atoi(size);

        while(allInit < acceptedClients);

        startSignal = true;
        if(throughput) {
            thread newTimerThread(setTimer, duration);
            newTimerThread.join();
        }
    }
    //cout << "waiting for threads" << endl;
    for(auto & clientThread : clientThreads){
        clientThread.join();
    }

    sendResultsMasterNode(masterSock, &results);

    for(int i = 0; i < clientFds.size(); i++) {
        close(clientFds.at(i));
    }
    close(sockFd);
    return 0;
}


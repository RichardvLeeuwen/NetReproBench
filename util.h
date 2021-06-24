

#ifndef CLOUDBENCHMARKING_UTIL_H
#define CLOUDBENCHMARKING_UTIL_H

#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <sys/socket.h>

using namespace std;

void sendResultsMasterNode(int masterSock, std::vector <unsigned long> *results) {
    unsigned int additionalChars = results->size() * 15;

    char done[]= "done,";
    char comma[] = ",";
    char end[] = ";";
    char endSignal[256  + additionalChars];
    strcpy(endSignal, done);


    for (long result : *results) {
        char averageInChar[15];
        sprintf(averageInChar, "%lu", result);
        strcat(endSignal, averageInChar);
        strcat(endSignal, comma);
    }

    strcat(endSignal, end);
    unsigned int bytesToSend = 256 + additionalChars;
    long sentBytes = 0;
    while(sentBytes < bytesToSend ) {
        sentBytes = sentBytes + send(masterSock, endSignal + sentBytes, bytesToSend - sentBytes, 0);
    }


    while(true) {
        fd_set readfdset;
        struct timeval tv;
        int retval;

        FD_ZERO(&readfdset);
        FD_SET(masterSock, &readfdset);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(masterSock+1, &readfdset, NULL, NULL, &tv);
        if (retval == -1) {
            perror("Select error, trying again");
        }
        else if (retval) {
            //cout << "got reply!\n";
            char masternodeReply[1024];
            recv(masterSock, masternodeReply, 1024, 0 );
            break;
        }
        else {
            printf("resending endsignal to masternode\n");
            char endSignalRetry[256 + additionalChars];
            strcpy(endSignalRetry, done);

            for (long result : *results) {
                char averageInChar[15];
                sprintf(averageInChar, "%lu", result);
                strcat(endSignalRetry, averageInChar);
                strcat(endSignalRetry, comma);
            }

            strcat(endSignalRetry, end);
            bytesToSend = 256 + additionalChars;
            sentBytes = 0;
            while (sentBytes < bytesToSend) {
                sentBytes = sentBytes + send(masterSock, endSignalRetry + sentBytes, bytesToSend - sentBytes, 0);
            }
        }
    }
}
void sendSettingsToReceiver(char* serverIP, char* clientIP, bool throughput, ssize_t packetSize, int clientFd) {
    try {
        char filename[128];
        char arrow[] = ">";
        strcpy(filename, serverIP);
        strcat(filename, arrow);
        strcat(filename, clientIP);
        char ltc[] = "ltcTest";

        char go[] = "go,";
        char comma[] = ",";

        char sizeChars[15];
        sprintf(sizeChars, "%zd", packetSize);
        char bufferWithSize[256];
        strcpy(bufferWithSize, go);
        strcat(bufferWithSize, sizeChars);
        strcat(bufferWithSize, comma);
        if(throughput) {
            strcat(bufferWithSize, filename);
        }
        else {
            strcat(bufferWithSize, ltc);
        }
        strcat(bufferWithSize, comma);

        int total = sizeof(bufferWithSize);
        int sent = 0;
        int received = 0;
        while(total > 0) {
            received = send(clientFd, bufferWithSize+sent, total, 0);
            total = total - received;
        }
    }
    catch (exception& e) {
        cout << e.what() << '\n';
    }
}

#endif //CLOUDBENCHMARKING_UTIL_H

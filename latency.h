//
// Created by richard on 09-03-21.
//

#ifndef LATENCY_H
#define LATENCY_H

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include "timer.h"

using namespace std;

ssize_t doLatencyTest(int duration, size_t packetSize, int receiverFD, char* clientIP, char* serverIP,  mutex &tlock) {
    ssize_t latencyresults[duration];
    ssize_t ret = 0;
    char* dataPacket = new char [packetSize];
    strcpy(dataPacket, "LTC");
    long long total = 0;
    int check = 1;
    for(int i = 0; i < duration ; i++) {
        long totalnanoseconds = 0;
        long nanoseconds = 0;
        long seconds = 0;
        struct timespec start=(struct timespec){0};
        struct timespec end= (struct timespec){0};
        clock_gettime(CLOCK_REALTIME, &start);
        check = send(receiverFD, dataPacket, packetSize, 0);
        if(check <= 0) {
            break;
        }
        fd_set readfdset;
        struct timeval tv;
        int retval;

        FD_ZERO(&readfdset);
        FD_SET(receiverFD, &readfdset);

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        retval = select(receiverFD+1, &readfdset, NULL, NULL, &tv);
        if (retval == -1) {
            perror("Select error, trying again");
        }
        else if (retval) {
            clock_gettime(CLOCK_REALTIME, &end);
            seconds = end.tv_sec - start.tv_sec;
            nanoseconds = end.tv_nsec - start.tv_nsec;
            totalnanoseconds = (seconds * 1000000000) + nanoseconds;
            latencyresults[i] = totalnanoseconds;
            total = total + latencyresults[i];
            continue;
        }
        else {
            i = i -1;
            continue;
        }
    }

    tlock.lock();
    string path = "thesis/cloudBenchResults/";
    char filename[128];
    char arr[] = ">";
    strcpy(filename, serverIP);
    strcat(filename, arr);
    strcat(filename, clientIP);
    string filePath = path + filename;
    ofstream logfile;
    logfile.open(filePath);
    for (ssize_t i : latencyresults) {
        logfile << i << '\n';
    }
    logfile.close();
    tlock.unlock();


    ret = total / duration;
    free(dataPacket);
    return ret;
}

void respondLatency(int receiverFD, size_t packetSize) {
    auto * dataPacket = new size_t [packetSize];
    int check = 1;
    while(true) {
        check = recv(receiverFD, dataPacket, packetSize, 0);
        if(check <= 0) {
            break;
        }
        check = send(receiverFD, dataPacket, packetSize, 0);
        if(check <= 0) {
            break;
        }
    }
    free(dataPacket);
}

#endif //LATENCY_H

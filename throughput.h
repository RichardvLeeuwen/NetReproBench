//
// Created by richard on 09-03-21.
//

#ifndef THROUGHPUT_H
#define THROUGHPUT_H

#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include "timer.h"

using namespace std;

ssize_t doThroughputTest(int duration, size_t packetSize, int receiverFD, char* clientIP, char* serverIP, mutex &tlock) {
    ssize_t *totalSendInBytes = new ssize_t ;
    ssize_t returnB = 0;
    ssize_t timestampedThroughput[duration];
    auto * dataPacket = new size_t [packetSize];
    time_t oldStamp = std::time(nullptr);
    int totalStamps = 1;
    try {
        while (!finished) {
            returnB = send(receiverFD, dataPacket, packetSize, 0);
            if (returnB <= 0) {
                cout << "oops" << endl;
                break;
            }
            *totalSendInBytes += returnB;
            time_t newStamp = std::time(nullptr);
            if (newStamp - oldStamp >= 1) {
                timestampedThroughput[totalStamps - 1] = *totalSendInBytes;
                oldStamp = time(nullptr);
                totalStamps = totalStamps + 1;
            }
        }
        char fin[4] = "fin";
        send(receiverFD, fin, sizeof(fin), 0);
        char recvBytes[120];

        while(true) {
            fd_set readfdset;
            struct timeval tv;
            int retval;

            FD_ZERO(&readfdset);
            FD_SET(receiverFD, &readfdset);

            tv.tv_sec = 1;
            tv.tv_usec = 0;

            retval = select(receiverFD+1, &readfdset, NULL, NULL, &tv);
            if (retval == -1) {
                perror("Select error, trying again");
            }
            else if (retval) {
                //cout << "got reply!\n";
                recv(receiverFD, recvBytes, sizeof(recvBytes), 0);
                break;
            }
            else {
                send(receiverFD, fin, sizeof(fin), 0);
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
        logfile << *totalSendInBytes << '\n';
        logfile << recvBytes << '\n';
        for (ssize_t i : timestampedThroughput) {
            logfile << i << '\n';
        }
        logfile.close();
        tlock.unlock();
//    long gigabitsec = (((((totalSendInBytes*8) / duration) /1024) /1024) /1024);
//    cout << "Transmitter sent " << totalSendInBytes << " bytes in " << time << " seconds." << endl;
//    cout << "This is equal to " << gigabitsec << " gigabit/second" << endl;

        free(dataPacket);
    }
    catch (exception& e) {
        cout << e.what() << '\n';
    }
    ssize_t ret = *totalSendInBytes;
    free(totalSendInBytes);
    return ret;
}

#endif //THROUGHPUT_H

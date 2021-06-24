//
// Created by richard on 09-03-21.
//

#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

volatile bool finished = false;

void setTimer(int seconds) {
    //cout << "Starting timer for " << seconds << " seconds." << endl;

    this_thread::sleep_for(chrono::seconds(seconds));
    finished = true;
}

#endif //TIMER_H

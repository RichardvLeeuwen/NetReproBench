

#ifndef CLOUDBENCHMARKING_SERVER_H
#define CLOUDBENCHMARKING_SERVER_H

#include <vector>
#include <string>

void handleNewClient(int clientFd, std::vector <unsigned long> *results, char* clientIP);

#endif //CLOUDBENCHMARKING_SERVER_H

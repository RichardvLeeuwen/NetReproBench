cmake_minimum_required(VERSION 3.17)
project(cloudBenchmarking)

set(CMAKE_CXX_STANDARD 20)


add_executable(server server.cpp server.h throughput.h timer.h latency.h util.h)
target_link_libraries(server pthread)

add_executable(client client.cpp client.h latency.h)
target_link_libraries(client pthread)
cmake_minimum_required(VERSION 3.8)
project(vpnServer)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp UdpServer.cpp UdpServer.h MyTun.cpp MyTun.h)

add_executable(vpnServer ${SOURCE_FILES})

target_link_libraries(vpnServer pthread)

target_link_libraries(vpnServer tins pcap)
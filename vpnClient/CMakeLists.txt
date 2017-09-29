cmake_minimum_required(VERSION 3.8)
project(vpnClient)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp UdpClient.cpp UdpClient.h MyTun.cpp MyTun.h)
add_executable(vpnClient ${SOURCE_FILES})
target_link_libraries(vpnClient pthread)
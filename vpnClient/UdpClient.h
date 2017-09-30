#ifndef VPNCLIENT_UDPCLIENT_H
#define VPNCLIENT_UDPCLIENT_H

#include <iostream>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>

class UdpClient
{
public:
    UdpClient(const char ip[], int port);

    void udpSend(uint8_t sendbuffer[], int &sendsize);

    int udpRecv(uint8_t recvbuffer[]);

    void udpClose();

    int clientfd;
    struct sockaddr_in servaddr;
};

#endif //VPNCLIENT_UDPCLIENT_H

#ifndef VPNSERVER_UDPSERVER_H
#define VPNSERVER_UDPSERVER_H

#include <iostream>
#include <netinet/in.h>
#include <unistd.h>

class UdpServer
{
public:
    UdpServer(int port);
    int udpRecv(uint8_t recvbuffer[]);
    void udpSend(uint8_t sendbuffer[], int &sendsize);
    void udpClose();

    int server_sockfd;
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
};


#endif //VPNSERVER_UDPSERVER_H

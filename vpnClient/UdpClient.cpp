#include "UdpClient.h"

UdpClient::UdpClient(const char ip[], int port)
{
    clientfd = socket(AF_INET,SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);
}

void UdpClient::udpSend(uint8_t sendbuffer[], int &sendsize)
{
    sendto(clientfd, sendbuffer, sendsize,0,(struct sockaddr *)&servaddr,sizeof(servaddr));
}

int UdpClient::udpRecv(uint8_t recvbuffer[])
{
    int num = recvfrom(clientfd, recvbuffer, 4096, 0, NULL, NULL);
    return num;
}

void UdpClient::udpClose()
{
    close(clientfd);
}
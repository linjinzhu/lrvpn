#include "UdpServer.h"

UdpServer::UdpServer(int port)
{
    struct sockaddr_in server_sockaddr;
    server_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    int opt = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(opt));
    if (bind(server_sockfd, (struct sockaddr *) &server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
}

int UdpServer::udpRecv(uint8_t *recvbuffer)
{
    int ret = recvfrom(server_sockfd, recvbuffer, 4096, 0, (struct sockaddr *) &clientaddr, &len);
    if (ret == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    return ret;
}

void UdpServer::udpSend(uint8_t *sendbuffer, int &sendsize, sockaddr_in sendClientaddr)
{
    int ret;
    ret = sendto(server_sockfd, sendbuffer, sendsize, 0, (struct sockaddr *) &sendClientaddr, sizeof(sendClientaddr));
    if (ret == -1)
    {
        perror("sendto");
        exit(1);
    }
}

void UdpServer::udpClose()
{
    close(server_sockfd);
}
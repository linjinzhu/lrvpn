#include <iostream>
#include <tins/tins.h>
#include <sys/epoll.h>
#include "UdpServer.h"
#include "MyTun.h"

using namespace std;

#define tunIPPrefix "192.168.2."
#define tunIPSeg "192.168.2.0/24"
#define PORT 16666
#define BUFFER_SIZE 2048
#define EPOLLEVENTS 1000

void handle(int epollfd,struct epoll_event *events,int num, MyTun &myTun, UdpServer &udpServer);
void udpHandle(MyTun &myTun, UdpServer &udpServer);
void tunHandle(MyTun &myTun, UdpServer &udpServer);
void add_event(int epollfd,int fd,int state);

struct ClientPro
{
    Tins::IPv4Address clientIP;
    sockaddr_in clientaddr;
    bool operator < (const ClientPro &rhs) const
    {
        return clientIP < rhs.clientIP;
    }
};

map<Tins::IPv4Address, ClientPro> ipMap;
map<ClientPro, Tins::IPv4Address> clientMap;
int clientNum = 1;
int main()
{
    MyTun myTun;
    UdpServer udpServer(PORT);
    string tunName = myTun.name;

    string cmd = "ifconfig " + tunName + " " + tunIPSeg + " up";
    system(cmd.c_str());

    int epollfd, ret;
    struct epoll_event events[EPOLLEVENTS];
    epollfd = epoll_create1(0);
    add_event(epollfd, udpServer.server_sockfd, EPOLLIN);
    add_event(epollfd, myTun.tunfd, EPOLLIN);

    while (1)
    {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle(epollfd, events, ret, myTun, udpServer);
    }

    return 0;
}

void handle(int epollfd,struct epoll_event *events,int num, MyTun &myTun, UdpServer &udpServer)
{
    int evfd;
    for (int i = 0; i < num; i++)
    {
        evfd = events[i].data.fd;
        if (events[i].events == EPOLLIN)
        {
            if (evfd == udpServer.server_sockfd)
            {
                udpHandle(myTun, udpServer);
            }
            else if (evfd == myTun.tunfd)
            {
                tunHandle(myTun, udpServer);
            }
        }
    }
}

void udpHandle(MyTun &myTun, UdpServer &udpServer)
{
    uint8_t buffer[BUFFER_SIZE];
    int recvNum;
    recvNum = udpServer.udpRecv(buffer);
    if (recvNum > 0)
    {

        Tins::IP sendIp;
        try
        {
            sendIp = Tins::RawPDU(buffer, recvNum).to<Tins::IP>();
        }
        catch (...)
        {
            return ;
        }
        ClientPro clientPro;
        clientPro.clientIP = sendIp.src_addr();
        clientPro.clientaddr = udpServer.clientaddr;

        if(clientMap.count(clientPro) == 0)
        {
            string tunIP = tunIPPrefix + to_string(clientNum);
            Tins::IPv4Address lo(tunIP);
            sendIp.src_addr(lo);

            clientMap[clientPro] = lo;
            ipMap[lo] = clientPro;
            clientNum++;
            if (clientNum == 254)
                clientNum = 1;
        }
        else
        {
            sendIp.src_addr(clientMap[clientPro]);
        }

        vector<uint8_t> writeIP = sendIp.serialize();
        int writeNum = writeIP.size();
        uint8_t writeBuf[BUFFER_SIZE];
        for (int i = 0; i < writeNum; ++i)
        {
            writeBuf[i] = writeIP[i];
        }

        myTun.tunWrite(writeBuf, writeNum);
    }
}

void tunHandle(MyTun &myTun, UdpServer &udpServer)
{
    uint8_t readBuf[BUFFER_SIZE];
    int readNum;
    readNum = myTun.tunRead(readBuf, BUFFER_SIZE);

    Tins::IP recvIp;
    try
    {
        recvIp = Tins::RawPDU(readBuf, readNum).to<Tins::IP>();
    }
    catch (...)
    {
        return ;
    }

    Tins::IPv4Address sendClientIP = ipMap[recvIp.dst_addr()].clientIP;
    sockaddr_in sendClientaddr = ipMap[recvIp.dst_addr()].clientaddr;
    recvIp.dst_addr(sendClientIP);

    vector<uint8_t> recvIP = recvIp.serialize();
    int recvNum = recvIP.size();
    uint8_t recvBuf[BUFFER_SIZE];
    for (int i = 0; i < recvNum; ++i)
    {
        recvBuf[i] = recvIP[i];
    }

    udpServer.udpSend(recvBuf, recvNum, sendClientaddr);
}

void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}
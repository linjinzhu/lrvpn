#include <iostream>
#include <tins/tins.h>
#include <sys/epoll.h>
#include "UdpServer.h"
#include "MyTun.h"

using namespace std;

#define tunIP "192.168.2.10"
#define tunIPSeg "192.168.2.0/24"
#define PORT 16666
#define BUFFER_SIZE 2048
#define EPOLLEVENTS 1000

void handle(int epollfd,struct epoll_event *events,int num, MyTun &myTun, UdpServer &udpServer);
void udpHandle(MyTun &myTun, UdpServer &udpServer);
void tunHandle(MyTun &myTun, UdpServer &udpServer);
void add_event(int epollfd,int fd,int state);

Tins::IPv4Address clientIP;

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
        Tins::IPv4Address lo(tunIP);
        clientIP = sendIp.src_addr();
        sendIp.src_addr(lo);

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
    recvIp.dst_addr(clientIP);

    vector<uint8_t> recvIP = recvIp.serialize();
    int recvNum = recvIP.size();
    uint8_t recvBuf[BUFFER_SIZE];
    for (int i = 0; i < recvNum; ++i)
    {
        recvBuf[i] = recvIP[i];
    }

    udpServer.udpSend(recvBuf, recvNum);
}

void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}
#include <iostream>
#include <linux/ip.h>
#include <sys/epoll.h>
#include <net/route.h>
#include "UdpClient.h"
#include "MyTun.h"
using namespace std;

#define BUF_SIZE 2048
#define EPOLLEVENTS 1000
#define PATH_ROUTE "/proc/net/route"

void handle(int epollfd,struct epoll_event *events,int num, MyTun &myTun, UdpClient &udpClient);
void tunHandle(MyTun &myTun, UdpClient &udpClient);
void udpHandle(MyTun &myTun, UdpClient &udpClient);
void add_event(int epollfd,int fd,int state);
bool GetIPV4Gateway(const char * pNICName, char *pGateway, unsigned long len);

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout << "Usage: ./vpnClient serverIP serverPort networkName" << endl;
        exit(0);
    }
    const char *serverIP = argv[1];
    int port = atoi(argv[2]);
    const char *networkName = argv[3];
    char gateway[32];
    GetIPV4Gateway(networkName, gateway, 32);

    MyTun myTun;
    UdpClient udpClient(serverIP, port);
    string tunName = myTun.name;

    string cmd = "route add " + (string) serverIP + " gw " + gateway;
    system(cmd.c_str());
    cmd = "ifconfig " + tunName + " mtu 1300 0.0.0.0 up";
    system(cmd.c_str());
    cmd = "ip route add default dev " + tunName;
    system(cmd.c_str());
    int epollfd, ret;
    struct epoll_event events[EPOLLEVENTS];
    epollfd = epoll_create1(0);
    add_event(epollfd, udpClient.clientfd, EPOLLIN);
    add_event(epollfd, myTun.tunfd, EPOLLIN);

    while (1)
    {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        handle(epollfd, events, ret, myTun, udpClient);
    }

    return 0;
}

void handle(int epollfd,struct epoll_event *events,int num, MyTun &myTun, UdpClient &udpClient)
{
    int evfd;
    for (int i = 0; i < num; i++)
    {
        evfd = events[i].data.fd;
        if (events[i].events == EPOLLIN)
        {
            if (evfd == myTun.tunfd)
            {
                tunHandle(myTun, udpClient);
            }
            else if (evfd == udpClient.clientfd)
            {
                udpHandle(myTun, udpClient);
            }
        }
    }
}

void tunHandle(MyTun &myTun, UdpClient &udpClient)
{
    uint8_t sendbuf[BUF_SIZE];
    int readNum;

    readNum = myTun.tunRead(sendbuf, BUF_SIZE);
    const void *peek = sendbuf;
    const struct iphdr *iphdr = static_cast<const struct iphdr *>(peek);
    if (iphdr->version == 4)
    {
        udpClient.udpSend(sendbuf, readNum);
    }
}

void udpHandle(MyTun &myTun, UdpClient &udpClient)
{
    uint8_t recvbuf[BUF_SIZE];
    int recvNum;

    recvNum = udpClient.udpRecv(recvbuf);
    myTun.tunWrite(recvbuf, recvNum);
}

void add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

bool GetIPV4Gateway(const char * pNICName, char *pGateway, unsigned long len)
{
    char buffer[200] = {0};
    unsigned long bufLen = sizeof(buffer);

    unsigned long defaultRoutePara[4] = {0};
    FILE *pfd = fopen(PATH_ROUTE, "r");
    if (NULL == pfd)
    {
        return false;
    }

    while (fgets(buffer, bufLen, pfd))
    {
        sscanf(buffer, "%*s %x %x %x %*x %*x %*x %x %*x %*x %*x\n", (unsigned int *) &defaultRoutePara[1],
               (unsigned int *) &defaultRoutePara[0], (unsigned int *) &defaultRoutePara[3],
               (unsigned int *) &defaultRoutePara[2]);

        if (NULL != strstr(buffer, pNICName))
        {
            //如果FLAG标志中有 RTF_GATEWAY
            if (defaultRoutePara[3] & RTF_GATEWAY)
            {
                unsigned long ip = defaultRoutePara[0];
                snprintf(pGateway, len, "%d.%d.%d.%d", (ip & 0xff), (ip >> 8) & 0xff, (ip >> 16) & 0xff,
                         (ip >> 24) & 0xff);
                break;
            }
        }

        memset(buffer, 0, bufLen);
    }

    fclose(pfd);
    pfd = NULL;
    return true;
}
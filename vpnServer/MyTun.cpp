#include <netinet/in.h>
#include <arpa/inet.h>
#include "MyTun.h"

MyTun::MyTun()
{
    struct ifreq ifr;

    if ((tunfd = open("/dev/net/tun", O_RDWR)) == -1)
    {
        perror("open");
        exit(errno);
    }
    bzero(&ifr, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, "", IFNAMSIZ);
    if (ioctl(tunfd, TUNSETIFF, &ifr) == -1)
    {
        perror("ioctl");
        exit(errno);
    }
    printf("ifname is %s\n", ifr.ifr_name);
    sprintf(name,"%s",ifr.ifr_name);
}

int MyTun::tunRead(uint8_t readbuf[], int readSize)
{
    if ((readNum = read(tunfd, readbuf, readSize)) == -1)
    {
        perror("read");
        exit(errno);
    }
    else if (readNum == 0)
    {
        exit(0);
    }
    return readNum;
}

void MyTun::tunWrite(uint8_t writebuf[], int writeSize)
{
    if ((writeNum = write(tunfd, writebuf, writeSize)) <= 0)
    {
        perror("write");
        exit(errno);
    }
}
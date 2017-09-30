#ifndef VPNCLIENT_MYTUN_H
#define VPNCLIENT_MYTUN_H

#include <iostream>
#include <linux/if_tun.h>
#include <net/if.h>     /* for struct ifreq */
#include <fcntl.h>
#include <cstring>     /* for bzero() */
#include <sys/ioctl.h>
#include <unistd.h>

class MyTun
{
public:
    MyTun();

    int tunRead(uint8_t readbuf[], int readSize);

    void tunWrite(uint8_t writebuf[], int writeSize);

    char name[10];
    int tunfd;
    int readNum;
    int writeNum;
};


#endif //VPNCLIENT_MYTUN_H

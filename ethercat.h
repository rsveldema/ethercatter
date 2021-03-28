#ifndef ETHECAT__H_H___
#define ETHECAT__H_H___

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/epoll.h>

#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <memory>
#include <array>
#include <functional>

#include "select.h"

#define ETH_P_ECAT              0x88A4

class EtherCatPacket
{
private:
    std::array<char, 2048> data;

public:
    void read(int fd);
};


class EtherCat
{
private:
    Select select;

public:
    EtherCat();

    ~EtherCat()
    {
    }

    void process_packet()
    {
        select.wait();
    }
};


#endif

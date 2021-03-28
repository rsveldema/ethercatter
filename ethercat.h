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

class EtherCatPacket
{
private:
    std::array<char, 2048> data;

public:
    void read(int fd)
    {
        auto ret = ::read(fd, data.data(), data.size());
        printf("read %d bytes\n", (int)ret);
    }
};

class Select
{
public:
    typedef std::function<void()> callback_t;

    Select()
    {
    }

    void add(int fd, callback_t&& callback);

    void wait();

private:
    std::vector<std::pair<int, callback_t>> fd_vals;
    int max_fd = 0;
};

class EtherCatQueue
{
private:
    static constexpr auto TUN_DEV = "/dev/net/tun";
    int fd = 0;
    std::string eth_src_dev;

public:
    EtherCatQueue(const EtherCatQueue &) = delete;
    EtherCatQueue operator=(const EtherCatQueue &) = delete;

    explicit EtherCatQueue(Select &select);
    int enable_queue(bool enable);

    ~EtherCatQueue()
    {
        close(fd);
    }
};

class EtherCat
{
private:
    std::vector<std::unique_ptr<EtherCatQueue>> queues;
    Select select;

public:
    EtherCat(unsigned num_queues);

    ~EtherCat()
    {
    }

    void process_packet()
    {
        select.wait();
    }
};


#endif

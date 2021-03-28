#include "ethercat.h"

void Select::add(int fd, callback_t &&callback)
{
    max_fd = std::max(fd, max_fd);
    fd_vals.push_back(std::make_pair(fd, std::move(callback)));
}

void Select::wait()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < fd_vals.size(); i++)
    {
        int fd = fd_vals[i].first;
        FD_SET(fd, &rfds);
    }
    fprintf(stderr, "start-select: %d\n", max_fd + 1);
    int ret = ::select(max_fd + 1, &rfds, NULL, NULL, NULL);
    if (ret < 0)
    {
        if (errno == EINTR)
        {
            fprintf(stderr, "EINTR\n");
            return;
        }

        perror("select()");
        return;
    }

    fprintf(stderr, "start-select-ret: %d\n", ret);
    for (int i = 0; i < fd_vals.size(); i++)
    {
        int fd = fd_vals[i].first;
        if (FD_ISSET(fd, &rfds))
        {
            fd_vals[i].second();
        }
    }
}

EtherCatQueue::EtherCatQueue(Select &select)
{
    if ((fd = open(TUN_DEV, O_RDWR)) < 0)
    {
        throw std::runtime_error("failed to open device");
    }

    select.add(fd, [this]() {
        fprintf(stderr, "processing packet\n");
        EtherCatPacket pkt;
        pkt.read(fd);
    });

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN - TUN device (no Ethernet headers) 
       *        IFF_TAP   - TAP device 
       *        IFF_NO_PI - Do not provide packet information  
       */
    ifr.ifr_flags = IFF_TAP | IFF_MULTI_QUEUE;
    //if (eth_src_dev != "")
    {
        strncpy(ifr.ifr_name,
                //"enp0s3",
                eth_src_dev.c_str(),
                IFNAMSIZ);
    }

    int err;
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0)
    {
        close(fd);
        throw std::runtime_error("failed to set tun name");
    }
    fprintf(stderr, "ethercat = %s\n", ifr.ifr_name);
    eth_src_dev = ifr.ifr_name;
}

int EtherCatQueue::enable_queue(bool enable)
{
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    if (enable)
        ifr.ifr_flags = IFF_ATTACH_QUEUE;
    else
        ifr.ifr_flags = IFF_DETACH_QUEUE;

    return ioctl(fd, TUNSETQUEUE, (void *)&ifr);
}

EtherCat::EtherCat(unsigned num_queues)
{
    for (auto i = 0; i < num_queues; i++)
    {
        auto aa = std::make_unique<EtherCatQueue>(select);
        queues.emplace_back(std::move(aa));
    }
}

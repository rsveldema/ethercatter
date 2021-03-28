#include "ethercat.h"
#include "netinet/in.h"

void EtherCatPacket::read(int fd)
{
    auto ret = ::read(fd, data.data(), data.size());
    printf("read %d bytes\n", (int)ret);

    struct ethhdr *eth = (struct ethhdr *)(data.data());
    printf("\nEthernet Header\n");
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    printf("\t|-Protocol : %d\n", eth->h_proto);
}


EtherCat::EtherCat()
{
    int sock_r = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ECAT));
    if (sock_r < 0)
    {
        perror("error in socket\n");
        return;
    }

    select.add(sock_r, [sock_r]() {
        EtherCatPacket pkt;
        pkt.read(sock_r);
    });
}

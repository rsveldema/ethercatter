#include "ethercat.h"
#include "netinet/in.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <netpacket/packet.h>

void mac_to_bin(const char* str, u_int8_t* data)
{
    const char *s = str;
    for (int i=0;i<6;i++)
    {
        char *end = nullptr;
        auto value = strtol(s, &end, 16);
        data[i] = value;

        //fprintf(stderr, "hex[%d] %x (%s)\n", i, (int)value, s);
        end++;


        s = end;
    }
}


void EtherCatPacket::create_heartbeat()
{
    struct ethhdr *eth = (struct ethhdr *)(data.data());

    const char*ME = "08:00:27:4f:08:67";
    const char*SRC = "07:00:26:4e:07:66";

    mac_to_bin(SRC, eth->h_source);  
    mac_to_bin(ME, eth->h_dest);
    //eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5];
    // eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5];

     eth->h_proto = htons(ETH_P_ECAT);

     len = 128;
}

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

void EtherCatPacket::write(int fd)
{
    static int ix = 0;
    fprintf(stderr, "sending: %d\n", ix++);
    ::write(fd, data.data(), len);
}

EtherCat::EtherCat()
{
    sock_raw = socket(AF_PACKET, SOCK_RAW,
         htons(ETH_P_ALL));
        // htons(ETH_P_ECAT));
    if (sock_raw < 0)
    {
        perror("error in socket\n");
        return;
    }

   struct timeval timeout;  
   struct ifreq ifr;
   struct sockaddr_ll sll;
   //struct sockaddr_ll sll;

    const char *ifname = "lo";


   timeout.tv_sec =  0;
   timeout.tv_usec = 1;
   int r = setsockopt(sock_raw, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
   r = setsockopt(sock_raw, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
   int i = 1;
   r = setsockopt(sock_raw, SOL_SOCKET, SO_DONTROUTE, &i, sizeof(i));
   /* connect socket to NIC by name */
   strcpy(ifr.ifr_name, ifname);
   r = ioctl(sock_raw, SIOCGIFINDEX, &ifr);
   auto ifindex = ifr.ifr_ifindex;
   strcpy(ifr.ifr_name, ifname);
   ifr.ifr_flags = 0;
   /* reset flags of NIC interface */
   r = ioctl(sock_raw, SIOCGIFFLAGS, &ifr);
   /* set flags of NIC interface, here promiscuous and broadcast */
   ifr.ifr_flags = ifr.ifr_flags | IFF_PROMISC | IFF_BROADCAST;
   r = ioctl(sock_raw, SIOCSIFFLAGS, &ifr);

   /* bind socket to protocol, in this case RAW EtherCAT */
   sll.sll_family = AF_PACKET;
   sll.sll_ifindex = ifindex;
   sll.sll_protocol = htons(ETH_P_ECAT);
   r = bind(sock_raw, (struct sockaddr *)&sll, sizeof(sll));

    select.add(sock_raw, [this]() {
        EtherCatPacket pkt;
        pkt.read(sock_raw);
    });
}

void EtherCat::slave()
{
    fprintf(stderr, "SLAVE\n");
    while (1)
    {
        process_packet();
    }
}

    

void EtherCat::send_heartbeat()
{
    EtherCatPacket pkt;
    pkt.create_heartbeat();

    pkt.write(sock_raw);
}

void EtherCat::master()
{
    fprintf(stderr, "MASTER\n");
    while (1)
    {
        ::sleep(1);
        send_heartbeat();
    }
}

#ifndef INQUISITOR
#define INQUISITOR

#define BUFFER_SIZE 1024
#define DEFAULT_ARP_FRAME_IF "eth0"

#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

struct arp_header
{
        uint16_t hardware_type;
        uint16_t protocol_type;
        uint8_t hardware_len;
        uint8_t protocol_len;
        uint16_t opcode;
        uint8_t sender_mac[6];
        uint8_t sender_ip[4];
        uint8_t target_mac[6];
        uint8_t target_ip[4];
} __attribute__((packed));

class Inquisitor
{
public:
        Inquisitor(const uint8_t *_target_mac, const uint8_t *_target_ipv4);
        Inquisitor(const Inquisitor &other);
        virtual ~Inquisitor();

        Inquisitor &operator=(const Inquisitor &other);

        bool send_arp_frame(const std::string &interface = DEFAULT_ARP_FRAME_IF);

        const int &get_sock_fd() const;
        const struct ifreq &get_if_idx() const;
        const struct ifreq &get_if_mac() const;
        const struct sockaddr_ll &get_sock_addr() const;
        const unsigned char *get_buffer() const;
        const struct ether_header *get_eth_header() const;
        const struct arp_header *get_arp_header() const;
        const uint8_t *get_target_mac() const;
        const uint8_t *get_target_ipv4() const;

        void set_sock_fd(const int &sock_fd);
        void set_if_idx(const struct ifreq &if_idx);
        void set_if_mac(const struct ifreq &if_mac);
        void set_sock_addr(const struct sockaddr_ll &sock_addr);
        void set_buffer(const unsigned char *buffer);
        void set_eth_header(const struct ether_header *eth_header);
        void set_arp_header(const struct arp_header *arp_header);
        void set_target_mac(const uint8_t *target_mac);
        void set_target_ipv4(const uint8_t *target_ipv4);

protected:
        int _sock_fd;
        struct ifreq _if_idx;
        struct ifreq _if_mac;
        struct sockaddr_ll _sock_addr;
        unsigned char _buffer[BUFFER_SIZE];
        struct ether_header *_eth_header;
        struct arp_header *_arp_header;
        uint8_t _target_mac[6];
        uint8_t _target_ipv4[4];

        bool _arp_frame_ready;

        Inquisitor();

        void _init();
        bool _init_arp_frame(const std::string &interface);

        bool _init_socket(const int &domain, const int &type, const int &protocol);
        bool _init_ifreq(struct ifreq &if_struct, const std::string &interface, const unsigned long &ioctl_request);
        bool _init_sock_addr_ll(const unsigned short &protocol);
        bool _init_eth_header(const uint16_t &frame_type);
        bool _init_arp_header();

        bool _send_frame() const;
};

#endif
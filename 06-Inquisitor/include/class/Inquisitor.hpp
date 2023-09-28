#ifndef INQUISITOR
#define INQUISITOR

#define BUFFER_SIZE 1024
#define DEFAULT_ARP_IF "eth0"
#define DEFAULT_ARP_HARDW_TYPE 1

#define ARP_REQUEST 1
#define ARP_REPLY 2

#include <iostream>
#include <cstring>
#include <errno.h>
#include <unistd.h>

#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <pcap.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <sstream>
#include <iomanip>

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

typedef struct arp_header t_arp_header;
typedef struct ifreq t_ifreq;
typedef struct sockaddr_ll t_sockaddr_ll;
typedef struct ether_header t_ether_header;
typedef struct ether_addr t_ether_addr;
typedef struct sockaddr t_sockaddr;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef struct pcap_pkthdr t_pcap_pkthdr;
typedef struct ip t_ip;
typedef struct arphdr t_arphdr;
typedef struct tcphdr t_tcphdr;
typedef struct udphdr t_udphdr;
typedef struct icmphdr t_icmphdr;
typedef struct in_addr t_in_addr;

class Inquisitor
{
public:
        Inquisitor(
            const uint8_t *src_mac,
            const uint8_t *src_ipv4,
            const uint8_t *target_mac,
            const uint8_t *target_ipv4);
        Inquisitor(const Inquisitor &other);
        virtual ~Inquisitor();

        Inquisitor &operator=(const Inquisitor &other);

        bool send_arp_frame();
        void listen_traffic(const bool &verbose);

        const uint8_t *get_src_mac() const;
        const uint8_t *get_src_ipv4() const;
        const uint8_t *get_target_mac() const;
        const uint8_t *get_target_ipv4() const;

        const int &get_sock_fd() const;
        const std::string &get_interface() const;
        const t_ifreq &get_if_idx() const;
        const t_ifreq &get_if_mac() const;
        const t_sockaddr_ll &get_sock_addr() const;
        const bool &get_socket_ready() const;

        const uchar *get_buffer() const;
        const t_ether_header *get_eth_header() const;
        const ushort &get_opcode() const;
        const ushort &get_hardware_type() const;
        const bool &get_spoofing() const;
        const t_arp_header *get_arp_header() const;
        const bool &get_arp_frame_ready() const;
        const pcap_t *get_pcap_hdl() const;

        void set_src_mac(const uint8_t *src_mac);
        void set_src_ipv4(const uint8_t *src_ipv4);
        void set_target_mac(const uint8_t *target_mac);
        void set_target_ipv4(const uint8_t *target_ipv4);

        void set_interface(const std::string &interface);
        void set_opcode(const ushort &opcode);
        void set_hardware_type(const ushort &hardware_type);
        void set_spoofing(const bool &spoofing);

protected:
        uint8_t _src_mac[6];
        uint8_t _src_ipv4[4];
        uint8_t _target_mac[6];
        uint8_t _target_ipv4[4];

        int _sock_fd;
        std::string _interface;
        t_ifreq _if_idx;
        t_ifreq _if_mac;
        t_sockaddr_ll _sock_addr;
        bool _socket_ready;

        uchar _buffer[BUFFER_SIZE];
        t_ether_header *_eth_header;
        ushort _opcode;
        ushort _hardware_type;
        bool _spoofing;
        t_arp_header *_arp_header;
        bool _arp_frame_ready;

        pcap_t *_pcap_hdl;

        Inquisitor();

        bool _set_socket();
        void _set_arp_frame();

        void _init();
        bool _init_socket(const int &domain, const int &type, const uint16_t &protocol);
        bool _init_ifreq(t_ifreq &if_struct, const unsigned long &ioctl_request);
        void _init_sockaddr_ll(const ushort &protocol);
        void _init_eth_header(const uint16_t &frame_type);
        void _init_arp_header();

        bool _send_frame();
        void _print_packet(const u_char *packet, const bpf_u_int32 &header_len, const bool &verbose);
        int _print_dll(const u_char *packet, const bool &verbose) const;
        uint8_t _print_nl(const u_char *packet, const uint16_t &protocol, const bool &verbose) const;
        size_t _print_tl(const u_char *packet, const uint8_t &protocol, const bool &verbose) const;
        void _print_al(const u_char *packet, const size_t &tl_size, const bpf_u_int32 &header_len, const bool &verbose) const;
        std::string _ifreq_to_mac_string() const;
        std::string _mac_to_string(const uchar *mac_address, const bool &add_zeros = true) const;
};

#endif
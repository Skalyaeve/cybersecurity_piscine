#include "../../include/class/Inquisitor.hpp"

Inquisitor::Inquisitor()
{
        this->_init();
        memset(this->_target_mac, 0, 6);
        memset(this->_target_ipv4, 0, 4);
}

Inquisitor::Inquisitor(const uint8_t *target_mac, const uint8_t *target_ipv4)
{
        this->_init();
        memcpy(this->_target_mac, target_mac, 6);
        memcpy(this->_target_ipv4, target_ipv4, 4);
}

Inquisitor::Inquisitor(const Inquisitor &other)
{
        *this = other;
}

Inquisitor::~Inquisitor()
{
        if (this->_sock_fd > 0)
                close(this->_sock_fd);
}

Inquisitor &Inquisitor::operator=(const Inquisitor &other)
{
        if (this != &other)
        {
                this->_sock_fd = other._sock_fd;
                this->_if_idx = other._if_idx;
                this->_if_mac = other._if_mac;
                this->_sock_addr = other._sock_addr;
                memcpy(this->_buffer, other._buffer, BUFFER_SIZE);
                memcpy(this->_eth_header, other._eth_header, sizeof(struct ether_header));
                memcpy(this->_arp_header, other._arp_header, sizeof(struct arp_header));
                memcpy(this->_target_mac, other._target_mac, 6);
                memcpy(this->_target_ipv4, other._target_ipv4, 4);
        }
        return *this;
}

bool Inquisitor::send_arp_frame(const unsigned short &opcode, const std::string &interface, const unsigned short &hardware_type)
{
        if (!this->_arp_frame_ready &&
            !this->_init_arp_frame(opcode, interface, hardware_type))
        {
                this->_init();
                return false;
        }
        return this->_send_frame();
}

void Inquisitor::_init()
{
        this->_sock_fd = 0;
        memset(&this->_if_idx, 0, sizeof(struct ifreq));
        memset(&this->_if_mac, 0, sizeof(struct ifreq));
        memset(&this->_sock_addr, 0, sizeof(struct sockaddr_ll));
        memset(this->_buffer, 0, BUFFER_SIZE);
        this->_eth_header = (struct ether_header *)this->_buffer;
        this->_arp_header = (struct arp_header *)(this->_buffer + sizeof(struct ether_header));

        this->_arp_frame_ready = false;
}

bool Inquisitor::_init_arp_frame(const unsigned short &opcode, const std::string &interface, const unsigned short &hardware_type)
{
        if (this->_init_socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL)) &&
            this->_init_self_ipv4(interface) &&
            this->_init_ifreq(this->_if_idx, interface, SIOCGIFINDEX) &&
            this->_init_ifreq(this->_if_mac, interface, SIOCGIFHWADDR) &&
            this->_init_sock_addr_ll(htons(ETH_P_ARP)) &&
            this->_init_eth_header(htons(ETH_P_ARP)) &&
            this->_init_arp_header(htons(opcode), htons(hardware_type)))
                this->_arp_frame_ready = true;
        return this->_arp_frame_ready;
}

bool Inquisitor::_init_socket(const int &domain, const int &type, const int &protocol)
{
        this->_sock_fd = socket(domain, type, protocol);
        if (this->_sock_fd < 0)
        {
                std::cerr << "Inquisitor: [ ERROR ] socket(" << domain << ", " << type << ", " << protocol << "): " << strerror(errno) << std::endl;
                return false;
        }
        return true;
}

bool Inquisitor::_init_self_ipv4(const std::string &interface)
{
        struct ifreq _if_ip;
        if (!this->_init_ifreq(_if_ip, interface, SIOCGIFADDR))
                return false;

    struct sockaddr_in* ip_addr = (struct sockaddr_in*)&ifr.ifr_addr;
    unsigned char *ip = (unsigned char *)&(ip_addr->sin_addr.s_addr);

    this->_sender_ipv4[0] = ip[0];
    this->_sender_ipv4[1] = ip[1];
    this->_sender_ipv4[2] = ip[2];
    this->_sender_ipv4[3] = ip[3];
        return true;
}

bool Inquisitor::_init_ifreq(struct ifreq &if_struct, const std::string &interface, const unsigned long &ioctl_request)
{
        strncpy(if_struct.ifr_name, interface.c_str(), IFNAMSIZ - 1);
        if (ioctl(this->_sock_fd, ioctl_request, &if_struct) < 0)
        {
                std::cerr << "Inquisitor: [ ERROR ] ioctl(" << this->_sock_fd << ", " << ioctl_request << ", " << &if_struct << "): " << strerror(errno) << std::endl;
                return false;
        }
        return true;
}

bool Inquisitor::_init_sock_addr_ll(const unsigned short &protocol)
{
        this->_sock_addr.sll_ifindex = this->_if_idx.ifr_ifindex;
        this->_sock_addr.sll_protocol = protocol;
        return true;
}

bool Inquisitor::_init_eth_header(const uint16_t &frame_type)
{
        this->_eth_header->ether_type = frame_type;
        memcpy(this->_eth_header->ether_shost, this->_if_mac.ifr_hwaddr.sa_data, 6);
        memcpy(this->_eth_header->ether_dhost, this->_target_mac, 6);
        return true;
}

bool Inquisitor::_init_arp_header(const uint16_t &opcode, const uint16_t &hardware_type)
{
        uint8_t e_target_mac[6];
        const uint8_t broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

        if (!memcmp(this->_target_mac, broadcast_mac, 6))
                memset(e_target_mac, 0, 6);
        else
                memcpy(e_target_mac, this->_target_mac, 6);

        arp->hardware_type = hardware_type;
        arp->protocol_type = htons(ETH_P_IP);
        arp->hardware_len = 6;
        arp->protocol_len = 4;
        arp->opcode = opcode;

        memcpy(arp->sender_mac, if_mac.ifr_hwaddr.sa_data, 6);
        arp->sender_ip[0] = 192;
        arp->sender_ip[1] = 168;
        arp->sender_ip[2] = 1;
        arp->sender_ip[3] = 2;
        memset(arp->target_mac, e_target_mac, 6);
        memset(arp->target_ip, this->_target_ipv4, 4);
        return true;
}

bool Inquisitor::_send_frame() const
{
}

const int &Inquisitor::get_sock_fd() const
{
        return this->_sock_fd;
}

const struct ifreq &Inquisitor::get_if_idx() const
{
        return this->_if_idx;
}

const struct ifreq &Inquisitor::get_if_mac() const
{
        return this->_if_mac;
}

const struct sockaddr_ll &Inquisitor::get_sock_addr() const
{
        return this->_sock_addr;
}

const unsigned char *Inquisitor::get_buffer() const
{
        return this->_buffer;
}

const struct ether_header *Inquisitor::get_eth_header() const
{
        return this->_eth_header;
}

const struct arp_header *Inquisitor::get_arp_header() const
{
        return this->_arp_header;
}

const uint8_t *Inquisitor::get_target_mac() const
{
        return this->_target_mac;
}

const uint8_t *Inquisitor::get_target_ipv4() const
{
        return this->_target_ipv4;
}

void Inquisitor::set_sock_fd(const int &sock_fd)
{
        this->_sock_fd = sock_fd;
}

void Inquisitor::set_if_idx(const struct ifreq &if_idx)
{
        this->_if_idx = if_idx;
}

void Inquisitor::set_if_mac(const struct ifreq &if_mac)
{
        this->_if_mac = if_mac;
}

void Inquisitor::set_sock_addr(const struct sockaddr_ll &sock_addr)
{
        this->_sock_addr = sock_addr;
}

void Inquisitor::set_buffer(const unsigned char *buffer)
{
        memcpy(this->_buffer, buffer, BUFFER_SIZE);
}

void Inquisitor::set_eth_header(const struct ether_header *eth_header)
{
        memcpy(this->_eth_header, eth_header, sizeof(struct ether_header));
}

void Inquisitor::set_arp_header(const struct arp_header *arp_header)
{
        memcpy(this->_arp_header, arp_header, sizeof(struct arp_header));
}

void Inquisitor::set_target_mac(const uint8_t *target_mac)
{
        memcpy(this->_target_mac, target_mac, 6);
}

void Inquisitor::set_target_ipv4(const uint8_t *target_ipv4)
{
        memcpy(this->_target_ipv4, target_ipv4, 4);
}

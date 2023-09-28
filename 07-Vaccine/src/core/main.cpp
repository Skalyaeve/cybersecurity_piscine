#include "../../include/header.hpp"

int main(int ac, char **av)
{
        if (ac < 2 || ac > 6)
        {
                std::cout << "Usage: " << av[0] << " [-X request_type] [-o archive_file] <url>" << std::endl;
                return 1;
        }
        const std::string url = av[ac - 1];
        std::string request_type = "GET";
        std::string archive_file = "archive.json";
        std::string temp;
        for (short i = 1; i < ac - 1; i++)
        {
                temp = av[i];
                if (temp == "-X" && ++i)
                {
                        temp = av[i];
                        if (temp != "GET" && temp != "POST")
                        {
                                std::cout << "Supported request types: GET, POST" << std::endl;
                                return 1;
                        }
                        request_type = temp;
                }
                else if (temp == "-o" && ++i)
                {
                        archive_file = av[i];
                        archive_file += ".json";
                }
                else
                {
                        std::cout << "Usage: " << av[0] << " [-X request_type] [-o archive_file] <url>" << std::endl;
                        return 1;
                }
        }
        std::ofstream file(archive_file);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open file: " << archive_file << std::endl;
                return 1;
        }
        file.close();
        Vaccine(url, request_type, archive_file).process();
        return 0;
}
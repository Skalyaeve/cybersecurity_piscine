#include "../../include/header.hpp"

bool parse_args(const int& ac, const char** av, std::string& archive_file, std::string& request_type, std::vector< std::string >& headers)
{
        std::string temp;
        for (size_t i = 1; i < ac - 1; i++)
        {
                temp = av[i];
                if (temp == "-o" && ++i)
                {
                        archive_file = av[i];
                        archive_file += ".json";
                }
                else if (temp == "-X" && ++i)
                {
                        temp = av[i];
                        if (temp != "GET" && temp != "POST")
                        {
                                std::cout << "Supported request types: GET, POST" << std::endl;
                                return false;
                        }
                        request_type = temp;
                }
                else if (temp == "-H" && ++i)
                {
                        temp = av[i];
                        size_t pos = temp.find('=');
                        if (pos == std::string::npos || pos == 0 || pos == temp.size() - 1)
                        {
                                std::cout << "Usage: " << av[0] << " [-X request_type] [-o archive_file] <url>" << std::endl;
                                return false;
                        }
                        headers.push_back(temp);
                }
                else
                {
                        std::cout << "Usage: " << av[0] << " [-X request_type] [-o archive_file] <url>" << std::endl;
                        return false;
                }
        }
        return true;
}

int main(int ac, char** av)
{
        if (ac < 2)
        {
                std::cout << "Usage: " << av[0] << " [-o archive_file] [-X request_type] [-H header_field=value] <url>" << std::endl;
                return 1;
        }
        const std::string url = av[ac - 1];
        std::string archive_file = "archive.json";
        std::string request_type = "GET";
        std::vector< std::string > headers;
        if (!parse_args(ac, av, archive_file, request_type, headers))
                return 1;

        std::ofstream file(archive_file);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open file: " << archive_file << std::endl;
                return 1;
        }
        file.close();

        Vaccine worker(url, archive_file, request_type, headers);
        if (worker.fetch_data())
        {
                std::cout << worker.get_html() << std::endl;
                std::cout << std::endl;
                std::cout << "Links count: " << worker.get_links().size() << std::endl;
                std::cout << "Forms count: " << worker.get_forms().size() << std::endl;
                // worker.union_based_sqli();
                // worker.error_based_sqli();
                // worker.boolean_based_sqli();
                // worker.time_based_sqli();
        }
        return 0;
}

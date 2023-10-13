#include "../../include/header.hpp"

bool parse_args(
    int& ac,
    char** av,
    std::string& archive_file,
    std::string& request_type,
    std::vector< std::string >& headers,
    short& db_type)
{
        std::string temp;
        for (int i = 1; i < ac - 1; i++)
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
                        size_t pos = temp.find(':');
                        if (pos == std::string::npos || pos == 0 || pos == temp.size() - 1)
                        {
                                std::cout << "USAGE: " << av[0] << " [OPTIONS] <URL>" << std::endl
                                          << "OPTIONS:" << std::endl
                                          << "-o <archive_file>" << std::endl
                                          << "-X <request_type>" << std::endl
                                          << "-H <header_field>=<value>" << std::endl
                                          << "-t <database>" << std::endl;
                                return false;
                        }
                        headers.push_back(temp);
                }
                else if (temp == "-t" && ++i)
                {
                        temp = av[i];
                        if (temp == "mysql")
                                db_type = 1;
                        else if (temp == "postgresql")
                                db_type = 2;
                        else if (temp == "sqlite")
                                db_type = 3;
                        else if (temp == "microsoftsql")
                                db_type = 4;
                        else
                        {
                                std::cout << "SUPPORTED DB:" << std::endl
                                          << "\tmysql" << std::endl
                                          << "\tpostgresql" << std::endl
                                          << "\tsqlite" << std::endl
                                          << "\tmicrosoftsql" << std::endl;
                                return false;
                        }
                }
                else
                {
                        std::cout << "USAGE: " << av[0] << " [OPTIONS] <URL>" << std::endl
                                  << "OPTIONS:" << std::endl
                                  << "\t-o <archive_file>" << std::endl
                                  << "\t-X <request_type>" << std::endl
                                  << "\t-H <header_field>=<value>" << std::endl
                                  << "\t-t <database>" << std::endl;
                        return false;
                }
        }
        return true;
}

int main(int ac, char** av)
{
        if (ac < 2)
        {
                std::cout << "USAGE: " << av[0] << " [OPTIONS] <URL>" << std::endl
                          << "OPTIONS:" << std::endl
                          << "\t-o <archive_file>" << std::endl
                          << "\t-X <request_type>" << std::endl
                          << "\t-H <header_field>=<value>" << std::endl
                          << "\t-t <database>" << std::endl;
                std::cout << "SUPPORTED DATABASES:" << std::endl
                          << "\tmysql" << std::endl
                          << "\tpostgresql" << std::endl
                          << "\tsqlite" << std::endl
                          << "\tmicrosoftsql" << std::endl;
                return 1;
        }
        const std::string url = av[ac - 1];
        std::string archive_file = "archive.vaccine";
        std::string request_type = "POST";
        short db_type = 0;
        std::vector< std::string > headers;
        if (!parse_args(ac, av, archive_file, request_type, headers, db_type))
                return 1;

        std::ofstream file(archive_file, std::ios_base::app);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open file: " << archive_file << std::endl;
                return 1;
        }
        file.close();

        Vaccine worker(url, archive_file, request_type, headers, db_type);
        if (worker.fetch_data())
        {
                std::cout << "=============== V A C C I N E ===============" << std::endl
                          << "Archive file: " << worker.get_archive_file() << std::endl
                          << "URL: " << worker.get_url() << std::endl
                          << "Request type: " << worker.get_request_type() << std::endl
                          << "Extra headers: " << (worker.get_headers().empty() ? "none" : "") << std::endl;
                for (const auto& header : worker.get_headers())
                        std::cout << "\t" << header << std::endl;
                std::cout << std::endl
                          << "Processing..." << std::endl
                          << std::endl;
                worker.union_based();
                std::cout << "=============================================" << std::endl;
        }
        return 0;
}

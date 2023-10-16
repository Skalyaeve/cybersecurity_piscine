#include "../../include/header.hpp"

bool parse_args(
    int& ac,
    char** av,
    short& db_type,
    std::string& request_type,
    std::vector< std::string >& headers,
    std::string& archive_file)
{
        std::string temp;
        for (int i = 1; i < ac - 1; i++)
        {
                temp = av[i];
                if (temp == "-o" && ++i)
                        archive_file = std::string(av[i]) + ".json";
                else if (temp == "-X" && ++i)
                {
                        temp = av[i];
                        if (temp != "GET" && temp != "POST")
                        {
                                std::cout << "Supported request types:" << std::endl
                                          << "GET" << std::endl
                                          << "POST" << std::endl;
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
                                          << "-H <header_field>:<value>" << std::endl
                                          << "-t <database>" << std::endl;
                                return false;
                        }
                        headers.push_back(temp);
                }
                else if (temp == "-t" && ++i)
                {
                        temp = av[i];
                        if (temp == "mysql")
                                db_type = MYSQL;
                        else if (temp == "postgresql")
                                db_type = POSTGRESQL;
                        else if (temp == "sqlite")
                                db_type = SQLITE;
                        else if (temp == "microsoftsql")
                                db_type = MICROSOFTSQL;
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
                                  << "\t-H <header_field>:<value>" << std::endl
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
                          << "\t-H <header_field>:<value>" << std::endl
                          << "\t-t <database>" << std::endl

                          << "SUPPORTED DATABASES:" << std::endl
                          << "\tmysql" << std::endl
                          << "\tpostgresql" << std::endl
                          << "\tsqlite" << std::endl
                          << "\tmicrosoftsql" << std::endl;
                return 1;
        }
        short db_type = ALL;
        const std::string url = av[ac - 1];
        std::string request_type = "GET";
        std::vector< std::string > headers;
        std::string archive_file = "archive.json";
        if (!parse_args(ac, av, db_type, request_type, headers, archive_file))
                return 1;

        // std::ofstream file(archive_file, std::ios_base::app);
        std::ofstream file(archive_file);
        if (!file.is_open())
        {
                std::cerr << "[ ERROR ] Could not open file: " << archive_file << std::endl;
                return 1;
        }
        file.close();

        Vaccine worker(url, request_type, headers, archive_file);
        if (worker.fetch_data())
        {
                std::cout << "=============== V A C C I N E ===============" << std::endl
                          << "Database type: " << worker.get_str_db_type() << std::endl
                          << "URL: " << worker.get_url() << std::endl
                          << "Request type: " << worker.get_request_type() << std::endl;
                if (!worker.get_headers().empty())
                {
                        std::cout << "Extra headers: " << std::endl;
                        for (const auto& header : worker.get_headers())
                                std::cout << "\t" << header << std::endl;
                }
                std::cout << "Archive file: " << worker.get_archive_file() << std::endl;
                worker.stacked_queries(db_type);
                worker.union_based(db_type);
                worker.error_based(db_type);
                worker.blind_based(db_type);
                std::cout << "=============================================" << std::endl;
        }
        return 0;
}

#include "../../include/header.hpp"

void print_usage()
{
        std::cout << "Usage: Stockholm [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -reverse, -r\t\t\tDecrypt mode." << std::endl;
        std::cout << "  -version, -v\t\t\tDisplay version." << std::endl;
        std::cout << "  -silent, -s\t\t\tSilent mode." << std::endl;
        std::cout << "  -help, -h\t\t\tDisplay this information." << std::endl;
}

int exit_success()
{
        std::cout << "---------------------------------------------------------" << std::endl;
        return 0;
}

int main(int ac, char **av)
{
        bool encrypt = true;
        bool verbose = true;
        const char *env_home = std::getenv("HOME");
        const std::string root = env_home ? std::string(env_home) : ".";
        Stockholm worker(&root, NULL, NULL);

        std::cout << "------------------| S T O C K H O L M |------------------" << std::endl;
        for (short i = 1; i < ac; i++)
        {
                const std::string arg = av[i];
                if (arg == "-reverse" || arg == "-r")
                        encrypt = false;
                else if (arg == "-silent" || arg == "-s")
                        verbose = false;
                else
                {
                        if (arg == "-help" || arg == "-h")
                                print_usage();
                        else if (arg == "-version" || arg == "-v")
                                worker.print_version();
                        else
                        {
                                std::cout << "[ERROR] Unknown option: " << arg << std::endl;
                                std::cout << "Try ./Stockholm -help" << std::endl;
                        }
                        return exit_success();
                }
        }
        if (encrypt)
                worker.encrypt(verbose);
        else
                worker.decrypt(verbose);
        return exit_success();
}
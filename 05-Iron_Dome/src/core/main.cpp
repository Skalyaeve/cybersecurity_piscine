#include "../../include/header.hpp"

int main(int ac, char **av)
{
        std::cout << "------------------| I R O N   D O M E |------------------" << std::endl;
        if (getuid())
                std::cout << "[ ERROR ] Program must be run as root." << std::endl;
        else
        {
                std::vector<std::string> workin_dirs;
                for (int i = 1; i < ac; ++i)
                        workin_dirs.push_back(std::string(av[i]));
                IronDome worker;
                if (workin_dirs.size())
                        worker.set_workin_dirs(workin_dirs);
                if (worker.launch())
                        std::cout << "[ ERROR ] fork() failed." << std::endl;
                else
                        std::cout << "[ SUCCESS ] Iron Dome daemon launched." << std::endl;
        }
        std::cout << "---------------------------------------------------------" << std::endl;
        return 0;
}
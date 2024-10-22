#include <iostream>
#include "builtin.hpp"

int shell_help(char **args)
{
    std::cout << "Shell from scratch" << std::endl;
    std::cout << "The following are built in:" << std::endl;

    for (int i = 0; i < shell_num_builtins(); i++)
    {
        std::cout << "  " << builtIn_string[i] << std::endl;
    }

    return 1;
}
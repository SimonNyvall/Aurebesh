#include <iostream>

int shell_write(char **args)
{
    if (args[1] == nullptr)
    {
        std::cerr << "mudsh: expected argument to \"write\"" << std::endl;
    }

    for (int i = 1; args[i] != nullptr; i++)
    {
        std::cout << args[i] << " ";
    }

    return 1;
}


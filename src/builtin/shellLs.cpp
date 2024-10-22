#include <iostream>
#include <string.h>

int shell_ls(char **args)
{
    std::string command = "ls --color=always -C";

    for (int i = 1; args[i] != nullptr; i++) 
    {
        command += " ";
        command += args[i];
    }

    FILE *file = popen(command.c_str(), "r");
    if (file == nullptr)
    {
        std::cerr << "Failed to run command" << std::endl;
        return 1;
    }

    char buffer[128];
    std::string result = "";

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);

    std::cout << result;

    return 1;
}
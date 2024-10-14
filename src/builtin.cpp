#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "shell.hpp"
#include "history/commandHistory.hpp"

const char *builtIn_string[] = {
    "cd",
    "exit",
    "help",
    "write",
    "history",
    "ls"};

int (*builtIn_string_func[])(char **) = { //* Function pointers (Do not forget to add the function pointer to shell.hpp)
    &shell_cd,
    &shell_exit,
    &shell_help,
    &shell_write,
    &shell_history,
    &shell_ls};

int shell_num_builtins()
{
    return sizeof(builtIn_string) / sizeof(char *);
}

int shell_cd(char **args)
{
    if (args[1] == nullptr)
    {
        std::cerr << "shell: expected argument to \"cd\"" << std::endl;
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int shell_exit(char **args)
{
    return 0;
}

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

int shell_history(char **args) //! Command is broken
{
    if (args[1] == nullptr)
    {
        std::cerr << "mudsh: expected argument to \"history\"" << std::endl;
    }

    if (strcmp(args[1], "list") == 0)
    {
        globalCommandHistory.printHistory();
    }

    return 1;
}

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

#include <iostream>
#include <stdio.h>
#include <unistd.h>

int shell_cd(char **args)
{
    if (args[1] == nullptr)
    {
        char *homePath = getenv("HOME");
        chdir(homePath);
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
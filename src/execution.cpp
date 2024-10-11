#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "shell.hpp"

int shellLaunch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            perror("mudsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("mudsh");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char **args)
{
    pid_t pid, wpid;
    int status;

    if (args[0] == nullptr)
    {
        return 0;
    }

    for (int i = 0; i < shell_num_builtins(); i++)
    {
        if (strcmp(args[0], builtIn_string[i]) == 0)
        {
            return builtIn_string_func[i](args);
        }
    }

    pid = fork();
    if (pid == 0)
    {
        // In the child process
        if (execvp(args[0], args) == -1)
        {
            perror("Error executing command");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("Error forking");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

int executePipeChain(char ***commands, int numCommands)
{
    int pipefd[2];
    pid_t pid;
    int prevPipeReadEnd = 0;
    for (int i = 0; i < numCommands; i++)
    {
        if (i < numCommands - 1)
        {
            if (pipe(pipefd) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();

        if (pid == 0)
        {
            if (i > 0)
            {
                dup2(prevPipeReadEnd, STDIN_FILENO);
            }
            if (i < numCommands - 1)
            {
                dup2(pipefd[1], STDOUT_FILENO);
            }

            close(pipefd[0]);
            execvp(commands[i][0], commands[i]);
            perror("mudsh");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("mudsh");
            exit(EXIT_FAILURE);
        }

        if (i > 0)
        {
            close(prevPipeReadEnd);
        }
        if (i < numCommands - 1)
        {
            close(pipefd[1]);
        }

        prevPipeReadEnd = pipefd[0];
        waitpid(pid, nullptr, 0);
    }

    return 1;
}

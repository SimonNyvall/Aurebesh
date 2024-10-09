#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include "shell.h"

int shellLaunch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("shell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("shell");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int execute(char **args) {
    if (args[0] == nullptr) {
        return 1;
    }

    for (int i = 0; i < shell_num_builtins(); i++) {
        if (strcmp(args[0], builtIn_string[i]) == 0) {
            return (*builtIn_string_func[i])(args);
        }
    }

    return shellLaunch(args);
}

int executePipeChain(char ***commands, int numCommands) {
    int pipefd[2], prevPipefd[2];
    pid_t pid;

    for (int i = 0; i < numCommands; i++) {
        if (i < numCommands - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        pid = fork();

        if (pid == 0) {
            if (i > 0) {
                dup2(prevPipefd[0], STDIN_FILENO);
                close(prevPipefd[0]);
            }

            if (i < numCommands - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            execvp(commands[i][0], commands[i]);
            perror("shell");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("shell");
            exit(EXIT_FAILURE);
        }

        if (i > 0) {
            close(prevPipefd[0]);
        }
        if (i < numCommands - 1) {
            close(pipefd[1]);
        }

        if (i > 0) {
            prevPipefd[0] = pipefd[0];
            prevPipefd[1] = pipefd[1];
        }

        waitpid(pid, nullptr, 0);
    }

    return 1;
}

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"

const char *builtIn_string[] = {
    "cd",
    "exit",
    "help"
};

int (*builtIn_string_func[])(char **) = {
    &shell_cd,
    &shell_exit,
    &shell_help
};

int shell_num_builtins() {
    return sizeof(builtIn_string) / sizeof(char *);
}

int shell_cd(char **args) {
    if (args[1] == nullptr) {
        std::cerr << "shell: expected argument to \"cd\"" << std::endl;
    } else {
        if (chdir(args[1]) != 0) {
            perror("shell");
        }
    }
    return 1;
}

int shell_exit(char **args) {
    return 0;
}

int shell_help(char **args) {
    std::cout << "Shell from scratch" << std::endl;
    std::cout << "The following are built in:" << std::endl;
    for (int i = 0; i < shell_num_builtins(); i++) {
        std::cout << "  " << builtIn_string[i] << std::endl;
    }
    return 1;
}

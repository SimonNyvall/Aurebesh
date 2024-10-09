#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "shell.h"

char *workingDirectory() {
    char *cwd = (char *)malloc(1024);
    getcwd(cwd, 1024);
    char *home = getenv("HOME");

    if (strstr(cwd, home) != nullptr) {
        return strstr(cwd, home) + strlen(home);
    }

    return cwd;
}

void shellLoop() {
    char *line;
    char ***commands;
    int numCommands;
    int status;

    do {
        std::cout << "\n" << workingDirectory() << " : ";
        line = readLine();

        commands = splitPipe(line, &numCommands);

        if (numCommands > 1) {
            status = executePipeChain(commands, numCommands);
        } else {
            status = execute(commands[0]);
        }

        for (int i = 0; i < numCommands; i++) {
            free(commands[i]); 
        }
        free(commands); 

        free(line); 
    } while (status);
}


int main() {
    shellLoop();
    return 0;
}

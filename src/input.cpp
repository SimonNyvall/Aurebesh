#include <iostream>
#include <cstdlib>
#include <cstring>
#include "shell.h"

char *readLine() {
    int bufSize = 1024;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * bufSize);
    int c;

    if (!buffer) {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;
        if (position >= bufSize) {
            bufSize += 1024;
            buffer = (char *)realloc(buffer, bufSize);
            if (!buffer) {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **splitLine(char *line) {
    int bufSize = 64;
    int position = 0;
    char **tokens = (char **)malloc(bufSize * sizeof(char *));
    char *token;

    if (!tokens) {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != nullptr) {
        tokens[position] = token;
        position++;

        if (position >= bufSize) {
            bufSize += 64;
            tokens = (char **)realloc(tokens, bufSize * sizeof(char *));
            if (!tokens) {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(nullptr, " \t\r\n\a");
    }

    tokens[position] = nullptr;
    return tokens;
}

char ***splitPipe(char *line, int *numCommands) {
    int bufSize = 64;  // Initial buffer size for commands
    int position = 0;  // Position in the commands array
    char ***commands = (char ***)malloc(bufSize * sizeof(char **));
    
    if (!commands) {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Split line by pipes and redirects
    char *command = strtok(line, "|>");
    while (command != nullptr) {
        commands[position] = splitLine(command);  // Split the command into tokens
        position++;

        if (position >= bufSize) {
            bufSize += 64;  // Increase buffer size
            commands = (char ***)realloc(commands, bufSize * sizeof(char **));
            if (!commands) {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        command = strtok(nullptr, "|>");  // Get the next command
    }

    commands[position] = nullptr;  // Null-terminate the commands array
    *numCommands = position;        // Set the number of commands
    return commands;
}

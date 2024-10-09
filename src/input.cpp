#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include "history/commandHistory.h"
#include "shell.h"
#include "promt/promt.h"

void enableRawMode(struct termios &orig_termios)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios &orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

char **splitLine(char *line)
{
    int bufSize = 64;
    int position = 0;
    char **tokens = (char **)malloc(bufSize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != nullptr)
    {
        tokens[position] = token;
        position++;

        if (position >= bufSize)
        {
            bufSize += 64;
            tokens = (char **)realloc(tokens, bufSize * sizeof(char *));
            if (!tokens)
            {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(nullptr, " \t\r\n\a");
    }

    tokens[position] = nullptr;
    return tokens;
}

char* concatenateCharArray(char **array) {
    std::size_t totalSize = 0;
    for (int i = 0; array[i] != nullptr; i++) {
        totalSize += strlen(array[i]) + 1;
    }

    char *result = (char*)malloc(totalSize * sizeof(char));
    if (!result) {
        std::cerr << "Memory allocation error." << std::endl;
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';

    for (int i = 0; array[i] != nullptr; i++) {
        strcat(result, array[i]);
        strcat(result, " ");
    }

    return result;
}


int handleEscChars(char *buffer, int *position)
{
    char seq[3];
    seq[0] = getchar();
    seq[1] = getchar();

    if (seq[0] == '[' && seq[1] == 'A')
    {
        char **lastCommand = globalCommandHistory.getCommand();

        if (!lastCommand)
        {
            return 0;
        }

        // clear the line and rewrite the current directory

        std::cout << "\r\033[K"; // Clear the line
        printInLinePromt();

        char *commandStr = concatenateCharArray(lastCommand);
        std::cout << commandStr;

        strcpy(buffer, commandStr);
        *position = strlen(commandStr);

        free(commandStr);

        return 0;
    }
}

char *readLine()
{
    int bufSize = 1024;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * bufSize);
    int c;
    struct termios orig_termios;

    enableRawMode(orig_termios);

    if (!buffer)
    {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == '\033') // ESC key
        {
            int result = handleEscChars(buffer, &position);

            if (result == 0)
            {
                continue;
            }
        }

        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';

            char *bufferCopy = strdup(buffer);
            char **command = splitLine(bufferCopy);
            globalCommandHistory.addCommand(command);

            free(bufferCopy);
            disableRawMode(orig_termios);

            std::cout << "\n";

            return buffer;
        }
        else
        {
            buffer[position] = c;
            std::cout << (char)c;
        }

        position++;

        if (position >= bufSize)
        {
            bufSize += 1024;
            buffer = (char *)realloc(buffer, bufSize);
            if (!buffer)
            {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }
}

char ***splitPipe(char *line, int *numCommands)
{
    int bufSize = 64;
    int position = 0;
    char ***commands = (char ***)malloc(bufSize * sizeof(char **));

    if (!commands)
    {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    char *start = line;
    char *pipePos;

    // Loop to find "|>" in the input line
    while ((pipePos = strstr(start, "|>")) != nullptr)
    {
        *pipePos = '\0';
        commands[position++] = splitLine(start);
        start = pipePos + 2;

        if (position >= bufSize)
        {
            bufSize += 64;
            commands = (char ***)realloc(commands, bufSize * sizeof(char **));
            if (!commands)
            {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    if (*start != '\0')
    {
        commands[position++] = splitLine(start);
    }

    commands[position] = nullptr;
    *numCommands = position;

    return commands;
}

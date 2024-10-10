#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <termios.h>
#include "history/commandHistory.hpp"
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

int handleEscChars(char *buffer, int *position, int *historyPosition)
{
    char seq[3];
    seq[0] = getchar();
    seq[1] = getchar();

    if (seq[0] != '[')
    {
        return 1;
    }

    if (seq[1] == 'A') // Up arrow
    {
        if (globalCommandHistory.size() == *historyPosition)
        {
            return 0;
        }

        *historyPosition = *historyPosition + 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return 0;
        }

        std::cout << "\r\033[K"; // Clear the current line
        printInLinePromt(); 

        std::cout << lastCommand;

        strncpy(buffer, lastCommand, 1024);
        buffer[1023] = '\0';
        *position = strlen(lastCommand); 

        for (int i = 0; i < *position; i++)
        {
            std::cout << "\b"; 
        }

        return 0;
    }

    if (seq[1] == 'B') // Down arrow
    {
        if (*historyPosition == 1)
        {
            std::cout << "\r\033[K"; // Clear the current line
            printInLinePromt();
            *historyPosition = *historyPosition - 1;

            return 0;
        }

        *historyPosition = *historyPosition - 1; 

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return 0;
        }

        std::cout << "\r\033[K"; // Clear the current line
        printInLinePromt();

        std::cout << lastCommand;

        strncpy(buffer, lastCommand, 1024);
        buffer[1023] = '\0';
        *position = strlen(lastCommand);

        for (int i = 0; i < *position; i++)
        {
            std::cout << "\b";
        }

        return 0;
    }

    if (seq[1] == 'D') // Left arrow
    {
        if (*position > 0) // Ensure the cursor cannot move past the promt line
        {
            std::cout << "\b"; // Move cursor left
            (*position)--;
        }
    }

    if (seq[1] == 'C') // Right arrow
    {
        if (*position < strlen(buffer))
        {
            std::cout << buffer[*position]; 
            (*position)++;
        }
    }

    return 1; 
}

char *readLine()
{
    int bufSize = 1024;
    int position = 0;
    char *buffer = (char *)malloc(sizeof(char) * bufSize);
    int c;
    struct termios orig_termios;
    int historyPosition = 0;

    enableRawMode(orig_termios);

    if (!buffer)
    {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        c = getchar();

        if (c == EOF || c == '\n') // Check for EOF or Enter key
        {
            buffer[position] = '\0';
            char *command = strdup(buffer);
            globalCommandHistory.addCommand(command);

            free(command);
            disableRawMode(orig_termios);

            historyPosition = 0;

            std::cout << "\n"; 
            return buffer; 
        }
        else if (c == '\033') // ESC key
        {
            handleEscChars(buffer, &position, &historyPosition);
        }
        else if (c == 127) // Backspace key
        {
            if (position > 0)
            {
                position--;
                buffer[position] = '\0';
                std::cout << "\b \b"; 
            }
        }
        else 
        {
            if (position < bufSize - 1) 
            {
                buffer[position] = c;
                std::cout << (char)c; 
                position++;
            }
        }

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

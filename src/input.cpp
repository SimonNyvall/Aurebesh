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
    char *currentToken = (char *)malloc(256);
    bool inQuotes = false;
    int currentPos = 0;

    if (!tokens || !currentToken)
    {
        std::cerr << "Allocation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int i = 0; line[i] != '\0'; i++)
    {
        char currentChar = line[i];

        if (currentChar == '"')
        {
            inQuotes = !inQuotes;
            continue;
        }

        if (inQuotes)
        {
            currentToken[currentPos++] = currentChar;
        }
        else
        {
            if (currentChar == ' ' || currentChar == '\t' || currentChar == '\n')
            {
                if (currentPos > 0)
                {
                    currentToken[currentPos] = '\0';
                    tokens[position++] = strdup(currentToken);
                    currentPos = 0;

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
                }
            }
            else
            {
                currentToken[currentPos++] = currentChar;
            }
        }

        if (currentPos >= 256)
        {
            std::cerr << "Token size exceeded, reallocating..." << std::endl;
            currentToken = (char *)realloc(currentToken, (currentPos + 1) * sizeof(char));
            if (!currentToken)
            {
                std::cerr << "Allocation error" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    if (currentPos > 0)
    {
        currentToken[currentPos] = '\0';
        tokens[position++] = strdup(currentToken);
    }

    if (inQuotes)
    {
        std::cerr << "Warning: Unclosed quotes detected." << std::endl;
    }

    tokens[position] = nullptr;
    free(currentToken);
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
            if (position == 0)
            {
                buffer[position] = '\0';
                std::cout << "\n";
                return nullptr;
            }

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
    if (!line)
    {
        return nullptr;
    }

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
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

    if (seq[0] != '[')
    {
        return 1;
    }

    if (seq[1] == 'A') // Up arrow
    {
        char **lastCommand = globalCommandHistory.getCommand();

        if (!lastCommand)
        {
            return 0;
        }

        std::cout << "\r\033[K"; // Clear the current line
        printInLinePromt(); 

        char *commandStr = concatenateCharArray(lastCommand);
        std::cout << commandStr;

        strncpy(buffer, commandStr, 1024);
        buffer[1023] = '\0';
        *position = strlen(commandStr); 

        for (int i = 0; i < *position; i++)
        {
            std::cout << "\b"; 
        }

        free(commandStr);
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
            char *bufferCopy = strdup(buffer);
            char **command = splitLine(bufferCopy);
            globalCommandHistory.addCommand(command);

            free(bufferCopy);
            disableRawMode(orig_termios);

            std::cout << "\n"; 
            return buffer; 
        }
        else if (c == '\033') // ESC key
        {
            handleEscChars(buffer, &position);
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

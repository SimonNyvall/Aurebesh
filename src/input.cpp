#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <termios.h>
#include "history/commandHistory.hpp"
#include "shell.hpp"
#include "promt/promt.hpp"

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

void refreshLine(const char *promt, std::string buffer, int position)
{
    std::cout << "\r\033[K"; // Clear the current line
    std::cout << promt << buffer;
    //std::cout.flush();

    int visiableLength = calculateVisiableLength(promt);

    int len = buffer.size();
    
    if (position < len)
    {
        for (int i = len; i > position; --i)
        {
            std::cout << "\b";
        }
    }
}

std::string handleEscChars(std::string buffer, int *position, int *historyPosition)
{
    char seq[3];
    seq[0] = getchar();
    seq[1] = getchar();

    if (seq[0] != '[')
    {
        return buffer;
    }

    if (seq[1] == 'A') // Up arrow
    {
        if (globalCommandHistory.size() == *historyPosition)
        {
            return buffer;
        }

        *historyPosition = *historyPosition + 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return buffer;
        }

        std::cout << "\r\033[K"; // Clear the current line
        printInLinePromt();

        std::cout << lastCommand;

        buffer = lastCommand;

        *position = buffer.size();

        return buffer;
    }

    if (seq[1] == 'B') // Down arrow
    {
        if (*historyPosition == 1) //? This should not clear the last command but give me the buffer written before accessing the history
        {
            std::cout << "\r\033[K"; // Clear the current line
            printInLinePromt();
            *historyPosition = *historyPosition - 1;

            return buffer;
        }

        *historyPosition = *historyPosition - 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return buffer;
        }

        std::cout << "\r\033[K"; // Clear the current line
        printInLinePromt();

        std::cout << lastCommand;

        buffer = lastCommand;

        *position = buffer.size();

        return buffer;
    }

    if (seq[1] == 'D') //* Left arrow
    {
        if (*position > 0) 
        {
            std::cout << "\033[D"; // Move cursor left
            (*position)--;
        }
    }

    if (seq[1] == 'C') // Right arrow
    {
        if (*position < buffer.size())
        {
            std::cout << buffer[*position];
            (*position)++;
        }
    }

    return buffer;
}

std::string readLine()
{
    std::string buffer; 
    int c;
    struct termios orig_termios;
    int position = 0;
    int historyPosition = 0;

    enableRawMode(orig_termios);

    std::string prompt = getInlinePromt();
    std::cout << prompt;

    while (true)
    {
        c = getchar();

        if (c == EOF || c == '\n') // Check for EOF or Enter key
        {
            if (buffer.empty())
            {
                std::cout << "\n";
                return ""; // Return an empty string instead of nullptr
            }

            char *command = strdup(buffer.c_str());
            globalCommandHistory.addCommand(command);

            std::cout << "\n";

            disableRawMode(orig_termios);

            historyPosition = 0;

            return buffer; 
        }
        else if (c == '\033') // ESC key
        {
            buffer = handleEscChars(buffer, &position, &historyPosition);
        }
        else if (c == 127) // Backspace key
        {
            if (!buffer.empty())
            {
                buffer.erase(position - 1, 1);
                position--;

                refreshLine(prompt.c_str(), buffer, position);
            }
        }
        else
        {
            buffer.insert(position, 1, static_cast<char>(c));
            position++;

            refreshLine(prompt.c_str(), buffer, position); 
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
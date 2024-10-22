#include <iostream>
#include <cstring>

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
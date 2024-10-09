#include "commandHistory.h"
#include <iostream>
#include <cstring>

CommandHistory globalCommandHistory(5);

CommandHistory::CommandHistory(int maxSize)
{
    this->maxSize = maxSize;
}

char **CommandHistory::copyCommand(char **command)
{
    int count = 0;

    while (command[count] != nullptr)
    {
        count++;
    }

    char **commandCopy = new char *[count + 1];

    for (int i = 0; i < count; i++)
    {
        commandCopy[i] = strdup(command[i]);
    }

    commandCopy[count] = nullptr;

    return commandCopy;
}

void CommandHistory::addCommand(char **command)
{
    char **commandCopy = copyCommand(command);

    if (history.size() == maxSize)
    {
        char **oldCommand = history.front();

        if (command != nullptr)
        {
            for (int i = 0; command[i] != nullptr; i++)
            {
                free(command[i]);
            }

            delete[] command;
        }

        history.pop_front();
    }

    history.push_back(commandCopy);
}

char **CommandHistory::getCommand()
{
    if (history.empty())
    {
        return nullptr;
    }

    return history.back();
}
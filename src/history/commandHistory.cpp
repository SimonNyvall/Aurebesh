#include "commandHistory.hpp"
#include <iostream>
#include <cstring>

CommandHistory globalCommandHistory(20);

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

void CommandHistory::addCommand(const char *command)
{
    if (history.size() >= maxSize) 
    {
        history.pop_front();
    }

    history.push_back(command);
}

const char *CommandHistory::getCommand(int position)
{
    if (position <= 0 || position > history.size())
    {
        return nullptr;
    }

    return history[history.size() - position].c_str();
}

void CommandHistory::printHistory()
{
    for (int i = 0; i < history.size(); i++)
    {
        std::cout << i + 1 << " " << history[i] << std::endl;
    }
}
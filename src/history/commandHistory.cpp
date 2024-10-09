#include "commandHistory.h"
#include <iostream>

CommandHistory globalCommandHistory(5);

CommandHistory::CommandHistory(int maxSize)
{
    this->maxSize = maxSize;
}

void CommandHistory::addCommand(char **command)
{
    if (history.size() == maxSize)
    {
        history.pop();
    }

    history.push(command);
}

char **CommandHistory::getCommand()
{
    if (history.empty())
    {
        return nullptr;
    }

    return history.top();
}

void CommandHistory::clearHistory()
{
    while (!history.empty())
    {
        delete[] history.top();
        history.pop();
    }
}

CommandHistory::~CommandHistory()
{
    clearHistory();
}
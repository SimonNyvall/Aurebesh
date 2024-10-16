#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <deque>
#include <cstring>
#include <string>

class CommandHistory
{
private:
    std::deque<std::string> history;
    int maxSize;

public:
    CommandHistory(int maxSize);
    char **copyCommand(char **command);
    void addCommand(const char *command);
    const char *getCommand(int position);
    void printHistory();
    int size();
};

extern CommandHistory globalCommandHistory;

#endif

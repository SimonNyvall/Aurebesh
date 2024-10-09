#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <deque>
#include <cstring>

class CommandHistory 
{
    private:
        std::deque<char **> history;
        int maxSize;

    public:
        CommandHistory(int maxSize);
        char **copyCommand(char **command);
        void addCommand(char **command);
        char **getCommand();
};

extern CommandHistory globalCommandHistory;

#endif

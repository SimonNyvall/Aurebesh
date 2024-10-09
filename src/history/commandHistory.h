#ifndef COMMAND_HISTORY_H
#define COMMAND_HISTORY_H

#include <stack>
#include <cstring>

class CommandHistory 
{
    private:
        std::stack<char **> history;
        int maxSize;

    public:
        CommandHistory(int maxSize);
        void addCommand(char **command);
        char **getCommand();
        void clearHistory();
        ~CommandHistory();
};

extern CommandHistory globalCommandHistory;

#endif

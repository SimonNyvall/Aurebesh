#include <string>

#ifndef PROMPT_H
#define PROMPT_H

class Prompt
{
public:
    std::string getPrompt();
    int length();
    void printPrompt();
    void updatePrompt();
    static Prompt &getInstance();

private:
    std::string promptText;
    static Prompt* promptInstance;
    Prompt();
};

#endif
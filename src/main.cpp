#include <iostream>
#include <cstring>
#include <unistd.h>
#include <string>
#include "shell.hpp"
#include "prompt/prompt.hpp"
#include "input/input.hpp"

void shellLoop()
{
    char ***commands;
    int numCommands;
    int status;

    do
    {
        std::string lineStr = readLine();

        if (lineStr.empty())
        {
            continue;
        }

        char *linePtr = new char[lineStr.size() + 1];
        strcpy(linePtr, lineStr.c_str());

        commands = splitPipe(linePtr, &numCommands);

        if (!commands)
        {
            continue;
        }

        if (numCommands > 1)
        {
            status = executePipeChain(commands, numCommands);
        }
        else
        {
            status = execute(commands[0]);
        }

        for (int i = 0; i < numCommands; i++)
        {
            free(commands[i]);
        }

        std::cout << "\n";

        free(commands);

        free(linePtr);
    } while (status);
}

int main()
{
    shellLoop();
    return 0;
}

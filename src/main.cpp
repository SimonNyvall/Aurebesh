#include <iostream>
#include <cstring>
#include <unistd.h>
#include "shell.h"
#include "promt/promt.h"

void shellLoop()
{
    char *line;
    char ***commands;
    int numCommands;
    int status;

    do
    {
        printNewLinePromt();
        
        line = readLine();
        commands = splitPipe(line, &numCommands);

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
        free(commands);

        free(line);
    } while (status);
}

int main()
{
    shellLoop();
    return 0;
}

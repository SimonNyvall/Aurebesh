#include <iostream>
#include "../history/commandHistory.hpp"

int shell_history(char **args)
{
    if (args[1] == nullptr)
    {
        std::cerr << "mudsh: expected argument to \"history\"" << std::endl;
    }

    if (strcmp(args[1], "list") == 0)
    {
        globalCommandHistory.printHistory();
    }

    return 1;
}
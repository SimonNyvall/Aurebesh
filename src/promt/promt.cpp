#include <iostream>
#include "promt.h"

void printNewLinePromt()
{
    std::cout << "\n" << workingDirectory() << " : ";
}

void printInLinePromt()
{
    std::cout << workingDirectory() << " : ";
}
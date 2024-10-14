#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <vector>

std::vector<std::string> tabCommandHandler(std::string buffer);
void printCommands(const std::vector<std::string> &commands);
std::string readLine();
char ***splitPipe(char *line, int *numCommands);

#endif
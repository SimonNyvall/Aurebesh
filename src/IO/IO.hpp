#ifndef IO_H
#define IO_H

#include <string>
#include <vector>
#include <filesystem>

/*
    * This function is used to handle the tab completion for the commands
    * 
    * @param buffer The current buffer
    * 
    * @return A vector of strings containing the possible commands
*/
std::vector<std::string> tabCommandHandler(std::string buffer, int cursorPosition);
void printCommands(const std::vector<std::string> &commands);

/*
    * This function is used to handle the tab completion for the cd command
    * 
    * @param buffer The current buffer
    * 
    * @return A vector of strings containing the possible paths
*/
std::vector<std::string> tabCdHandler(std::string buffer);
void printCdPaths(const std::vector<std::string> &paths);


bool isExecutable(const std::filesystem::path &path);
std::string toLower(const std::string &str);
std::string findComonPrefix(const std::vector<std::string>& strings);


std::string readLine();
char ***splitPipe(char *line, int *numCommands);

#endif
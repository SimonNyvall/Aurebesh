#include <string>

#ifndef PROMT_H
#define PROMT_H

void printInLinePrompt();
void printNewLinePrompt();
std::string getInlinePrompt();
std::string getPrompt();
int calculateVisiableLength(const std::string &str);

#endif
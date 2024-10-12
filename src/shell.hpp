#include <string>

#ifndef SHELL_H
#define SHELL_H

// Function declarations for built-in shell commands:
int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_write(char **args);
int shell_history(char **args);
int shell_ls(char **args);

// Header functions
std::string readLine();
char **splitLine(char *line);
char ***splitPipe(char *line, int *numCommands);
int executePipeChain(char ***commands, int numCommands);
int shellLaunch(char **args);
int execute(char **args);
void shellLoop();

extern const char *builtIn_string[];
extern int (*builtIn_string_func[])(char **);
int shell_num_builtins();

#endif

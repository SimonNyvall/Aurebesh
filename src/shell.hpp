#ifndef SHELL_H
#define SHELL_H

int executePipeChain(char ***commands, int numCommands);
int shellLaunch(char **args);
int execute(char **args);
void shellLoop();

#endif

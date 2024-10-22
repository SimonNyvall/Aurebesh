#ifndef BUILTIN_H
#define BUILTIN_H

int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_write(char **args);
int shell_history(char **args);
int shell_ls(char **args);

extern const char *builtIn_string[];
extern int (*builtIn_string_func[])(char **);
int shell_num_builtins();

#endif


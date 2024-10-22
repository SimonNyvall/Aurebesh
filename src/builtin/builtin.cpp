#include "builtin.hpp"

const char *builtIn_string[] = {
    "cd",
    "exit",
    "help",
    "write",
    "history",
    "ls"};

int (*builtIn_string_func[])(char **) = { //* Function pointers (Do not forget to add the function pointer to shell.hpp)
    &shell_cd,
    &shell_exit,
    &shell_help,
    &shell_write,
    &shell_history,
    &shell_ls};

int shell_num_builtins()
{
    return sizeof(builtIn_string) / sizeof(char *);
}
#include <cstdlib>
#include <cstring>
#include <unistd.h>

char *workingDirectory()
{
    char *cwd = (char *)malloc(1024);
    getcwd(cwd, 1024);
    char *home = getenv("HOME");

    if (strstr(cwd, home) != nullptr)
    {
        return strstr(cwd, home) + strlen(home);
    }

    return cwd;
}
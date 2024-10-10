#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cstdio>
#include <sys/stat.h>
#include <vector>

const std::string resetCode = "\033[0m";
const std::string lightBlueCode = "\033[1;34m";
const std::string redCode = "\033[0;31m";
const std::string purpleCode = "\033[0;35m";
const std::string boldCode = "\033[1m";

bool isInGitRepository()
{
    FILE *file = popen("git rev-parse --is-inside-work-tree 2>/dev/null", "r");
    char buffer[128];
    std::string result = "";

    while (fgets(buffer, 128, file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);

    return result == "true\n";
}

std::string currentGitBranchName()
{
    FILE *file = popen("git branch --show-current 2>/dev/null", "r");
    char buffer[128];
    std::string result = "";

    while (fgets(buffer, 128, file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

std::string workingDirectory()
{
    char *cwd = (char *)malloc(1024);
    if (getcwd(cwd, 1024) == nullptr)
    {
        perror("getcwd() error");
        free(cwd);
        return "";
    }

    char *home = getenv("HOME");

    if (home != nullptr && strstr(cwd, home) != nullptr)
    {
        std::string result = strstr(cwd, home) + strlen(home);
        std::string homePath = std::string(home) + "/";
        if (result.empty())
        {
            free(cwd);
            return "~";
        }
        if (result[0] == '/')
        {
            result = result.substr(1);
        }
        free(cwd);
        return "~/" + result;
    }

    std::string result = cwd;
    free(cwd);
    return result;
}

std::string workingDirectoryFromGit()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        perror("getcwd() error");
        return "";
    }

    std::string currentDir = cwd;
    std::string gitRoot = currentDir;

    while (true)
    {
        struct stat buffer;
        if (stat((gitRoot + "/.git").c_str(), &buffer) == 0)
        {
            break;
        }

        std::size_t pos = gitRoot.find_last_of("/");
        if (pos == std::string::npos)
        {
            return "Not in a Git repository";
        }

        gitRoot = gitRoot.substr(0, pos);
    }

    std::string relativePath = currentDir.substr(gitRoot.length());

    if (!relativePath.empty() && relativePath[0] == '/')
    {
        relativePath = relativePath.substr(1);
    }

    std::string parentDir = gitRoot.substr(gitRoot.find_last_of("/") + 1);

    if (relativePath.empty())
    {
        return parentDir;
    }

    return parentDir + "/" + relativePath;
}

bool isGitAhead()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && result.find("ahead") != std::string::npos;
}

bool isGitBehind()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && result.find("behind") != std::string::npos;
}

bool hasGitUntrackedChanges()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && result.find("??") != std::string::npos;
}

bool hasGitStagedChanges()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && (result.find("A") != std::string::npos || result.find("M") != std::string::npos);
}

bool hasGitChanges()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && (result.find("M") != std::string::npos || result.find("??") != std::string::npos);
}

bool hasGitDeletedLines()
{
    FILE *file = popen("git status -sb 2>/dev/null", "r");
    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), file) != nullptr)
    {
        result += buffer;
    }

    pclose(file);
    return !result.empty() && result.find("D") != std::string::npos;
}

std::string gitStatus()
{
    std::vector<std::string> status;

    if (hasGitChanges())
    {
        status.push_back("!");
    }

    if (hasGitStagedChanges())
    {
        status.push_back("+");
    }

    if (hasGitUntrackedChanges())
    {
        status.push_back("?");
    }

    if (hasGitDeletedLines())
    {
        status.push_back("✘");
    }

    if (isGitAhead())
    {
        status.push_back("⇡");
    }

    if (isGitBehind())
    {
        status.push_back("⇣");
    }

    if (status.empty())
    {
        return "";
    }

    std::string result;

    for (const auto& s : status)
    {
        result += s;
    }

    return " [" + result + "]";
}

void printNewLinePromt()
{
    if (isInGitRepository())
    {
        std::cout << "\n"
                  << lightBlueCode
                  << workingDirectoryFromGit()
                  << resetCode
                  << " on "
                  << purpleCode
                  << " "
                  << currentGitBranchName()
                  << resetCode
                  << redCode
                  << boldCode
                  << gitStatus()
                  << resetCode
                  << " ~> ";
        return;
    }

    std::cout << "\n"
              << lightBlueCode << workingDirectory() << resetCode << " ~> ";
}

void printInLinePromt()
{
    if (isInGitRepository())
    {
        std::cout << lightBlueCode
                  << workingDirectoryFromGit()
                  << resetCode
                  << " on "
                  << purpleCode
                  << " "
                  << currentGitBranchName()
                  << resetCode
                  << redCode
                  << boldCode
                  << gitStatus()
                  << resetCode
                  << " ~> ";
        return;
    }

    std::cout << lightBlueCode << workingDirectory() << resetCode << " ~> ";
}
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <regex>
#include "IO.hpp"
#include <asm-generic/ioctls.h>
#include <sys/ioctl.h>
#include "../errorLog/errorLog.hpp"

std::vector<std::string> getSystemCommands()
{
    std::vector<std::string> commands;

    char *pathEnv = std::getenv("PATH");

    if (pathEnv == nullptr)
    {
        return commands;
    }

    std::string pathEnvStr = pathEnv;
    std::vector<std::string> paths;

    std::size_t position = 0;

    while ((position = pathEnvStr.find(':')) != std::string::npos)
    {
        paths.push_back(pathEnvStr.substr(0, position));
        pathEnvStr.erase(0, position + 1);
    }

    paths.push_back(pathEnvStr);

    for (const auto &dir : paths)
    {
        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(dir))
            {
                if (isExecutable(entry.path()))
                {
                    commands.push_back(entry.path().filename().string());
                }
            }
        }
        catch (const std::exception &e)
        {
            logErrorToFile("tabCommandHandler.cpp", "ERROR", e.what());
        }
    }

    return commands;
}

std::vector<std::string> getLocalCommands(std::string path)
{
    std::vector<std::string> commands;

    std::filesystem::path fsPath(path);

    if (fsPath.is_relative())
    {
        fsPath = std::filesystem::absolute(fsPath);
    }

    if (!std::filesystem::is_directory(fsPath))
    {
        return commands;
    }

    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(fsPath))
        {
            if (std::filesystem::is_directory(entry.path()))
            {
                commands.push_back(entry.path().filename().string() + "/");
            }
            else
            {
                commands.push_back(entry.path().filename().string());
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        logErrorToFile("tabCommandHandler.cpp", "ERROR", e.what());
    }

    return commands;
}

std::vector<std::string> tabCommandHandler(std::string buffer, int cursorPosition)
{
    std::vector<std::string> commands;
    std::string bufferPart = buffer.substr(0, cursorPosition);

    std::regex pathRegex(R"(\.\.\/[^ ]+|\.\.\/|\.\/[^ ]+|\.\/|[^ ]+\/[^ ]*)");
    std::sregex_iterator it(bufferPart.begin(), bufferPart.end(), pathRegex);
    std::sregex_iterator end;

    std::smatch closestMatch;

    if (std::regex_search(bufferPart, closestMatch, pathRegex))
    {
        std::string pathPart;
        while (it != end)
        {
            std::smatch match = *it;
            if (match.position(0) + match.length(0) == bufferPart.length())
            {
                pathPart = match.str(0);
                closestMatch = match;
                break;
            }

            ++it;
        }

        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path fullPath = currentPath / pathPart;

        commands = getLocalCommands(fullPath.parent_path().string());

        std::string currentCommand = fullPath.filename().string();
        std::string currentCommandLower = toLower(currentCommand);
        std::vector<std::string> matchingCommands;

        for (const auto &command : commands)
        {
            std::string commandPart = command;
            std::string commandPartLower = toLower(commandPart);

            if (commandPartLower.rfind(currentCommandLower, 0) == 0)
            {
                std::size_t lastSlashPos = buffer.find_last_of('/');
                std::string basePath = (lastSlashPos != std::string::npos) ? buffer.substr(0, lastSlashPos + 1) : buffer;

                matchingCommands.push_back(basePath + commandPart);
            }
        }

        if (!matchingCommands.empty())
        {
            std::string commonPrefix = findComonPrefix(matchingCommands);

            if (buffer == commonPrefix && !commands.empty())
            {
                printCommands(matchingCommands);
            }

            if (!commonPrefix.empty())
            {
                return {commonPrefix};
            }

            return matchingCommands;
        }
    }
    else
    {
        commands = getSystemCommands();
    }

    if (commands.empty())
    {
        return std::vector<std::string>();
    }

    std::vector<std::string> matchingCommands;
    std::string bufferLower = toLower(buffer);

    for (const auto &command : commands)
    {
        std::string commandLower = command;

        if (commandLower.rfind(bufferLower, 0) == 0)
        {
            matchingCommands.push_back(command + " ");
        }
    }

    if (matchingCommands.size() == 0)
    {
        return commands;
    }

    return matchingCommands;
}

void printCommands(const std::vector<std::string> &commands)
{
    if (commands.empty())
    {
        std::cout << '\n' << "These are not the files you are looking for..." << "\n\n";
        return;
    }

    if (commands.size() > 100)
    {
        std::cout << '\n'
                  << "Display all " << commands.size() << " possibilities? (y/n)";

        char response;

        std::cin >> response;

        if (response != 'y')
        {
            std::cout << "\n\n";
            return;
        }
    }

    std::size_t maxLength = 0;
    for (const auto &command : commands)
    {
        if (command.length() > maxLength)
        {
            maxLength = command.length();
        }
    }

    const int COLUMN_WIDTH = maxLength + 2;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    const int COMMANDS_PER_LINE = w.ws_col / COLUMN_WIDTH;
    int count = 0;

    std::cout << "\n";

    bool isNewLine = false;
    for (const auto &command : commands)
    {
        std::string toPrint = command;

        std::size_t lastSlashPos = command.find_last_of('/');
        if (lastSlashPos != std::string::npos)
        {
            if (lastSlashPos == command.length() - 1)
            {
                lastSlashPos = command.find_last_of('/', lastSlashPos - 1);
            }

            toPrint = command.substr(lastSlashPos + 1);
        }

        if (isNewLine)
        {
            std::cout << '\n';
            isNewLine = false;
        }

        std::cout << std::setw(COLUMN_WIDTH) << std::left << toPrint;

        count++;
        if (count % COMMANDS_PER_LINE == 0)
        {
            isNewLine = true;
        }
    }

    std::cout << "\n\n";
}
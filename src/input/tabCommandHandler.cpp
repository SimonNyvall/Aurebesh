#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <iomanip>
#include <algorithm>

bool isExecutable(const std::filesystem::path &path)
{
    return std::filesystem::is_regular_file(path) && access(path.c_str(), X_OK) == 0;
}

std::vector<std::string> getPATHCommands()
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
            std::cerr << '\n'
                      << "Error accessing directory " << dir << e.what() << '\n';
        }
    }

    return commands;
}

std::vector<std::string> getCommandsFromPath(std::string path)
{
    std::vector<std::string> commands;

    // Debug print: original path

    // Resolve the path to the filesystem
    std::filesystem::path fsPath(path);

    // If the path is relative, resolve it to an absolute path
    if (fsPath.is_relative())
    {
        fsPath = std::filesystem::absolute(fsPath);
        // Debug print: resolved absolute path
    }

    // Check if the path is a directory
    if (!std::filesystem::is_directory(fsPath))
    {
        return commands; // Return an empty vector if it's not a directory
    }

    try
    {
        // Debug print: iterating over directory
        // Use std::filesystem to iterate over the specified directory
        for (const auto &entry : std::filesystem::directory_iterator(fsPath))
        {
            // Add both directories and executables to the list
            if (std::filesystem::is_directory(entry.path()))
            {
                commands.push_back(entry.path().filename().string() + "/"); // Add a '/' to indicate a directory
            }
            else if (isExecutable(entry.path())) // Check if the entry is an executable
            {
                commands.push_back(entry.path().filename().string()); // Store the executable's name
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error accessing directory " << path << ": " << e.what() << '\n';
    }

    return commands;
}

std::vector<std::string> tabCommandHandler(std::string buffer)
{
    std::vector<std::string> commands;

    if (buffer.find('/') != std::string::npos || buffer.find("..") != std::string::npos || buffer.find('.') != std::string::npos)
    {
        std::filesystem::path currentDir = std::filesystem::current_path();
        std::filesystem::path fullPath = currentDir / buffer;

        commands = getCommandsFromPath(fullPath.string());
    }
    else
    {
        commands = getPATHCommands();
    }

    if (commands.empty())
    {
        return std::vector<std::string>();
    }

    std::string currentCommand = buffer.substr(buffer.find_last_of('/') + 1); //! Something here
    std::cout << "Current command: " << currentCommand << std::endl;

    std::vector<std::string> matchingCommands;

    for (const auto &command : commands)
    {

        std::cout << command << std::endl;
        if (command.rfind(currentCommand, 0) == 0)
        {
            // buffer = command + " ";
            // break;

            matchingCommands.push_back(command);
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
    const int COMMANDS_PER_LINE = 5;
    int count = 0;

    std::cout << "\n";

    bool isNewLine = false;
    for (const auto &command : commands)
    {
        if (isNewLine)
        {
            std::cout << '\n';
            isNewLine = false;
        }

        std::cout << std::setw(COLUMN_WIDTH) << std::left << command;

        count++;
        if (count % COMMANDS_PER_LINE == 0)
        {
            isNewLine = true;
        }
    }

    std::cout << "\n\n";
}
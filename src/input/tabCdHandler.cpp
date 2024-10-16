#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include "input.hpp"

std::vector<std::string> getDirectoriesCommands(const std::filesystem::path &path)
{
    std::vector<std::string> dirs;

    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(path))
        {
            if (std::filesystem::is_directory(entry.path()))
            {
                dirs.push_back(entry.path().filename().string() + "/");
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return dirs;
}

std::vector<std::string> tabCdHandler(std::string buffer)
{
    int cdPos = buffer.find("cd ");
    if (cdPos == std::string::npos)
    {
        return std::vector<std::string>();
    }

    auto bufferPath = buffer.substr(cdPos + 3);

    std::filesystem::path currentPath = std::filesystem::current_path();
    std::filesystem::path fullPath = currentPath / bufferPath;

    std::string currentBufferPath = fullPath.filename().string();
    std::string currentBufferPathLower = toLower(currentBufferPath);
    std::vector<std::string> matchingPaths;

    std::vector<std::string> dirs = getDirectoriesCommands(fullPath.parent_path());

    for (const auto &path : dirs)
    {
        std::string pathPart = path;
        std::string pathPartLower = toLower(pathPart);

        if (pathPartLower.rfind(currentBufferPathLower, 0) == 0)
        {
            std::size_t lastSlashPos = bufferPath.find_last_of('/');
            std::string basePath = (lastSlashPos != std::string::npos) ? bufferPath.substr(0, lastSlashPos + 1) : pathPart;

            if (basePath == pathPart)
            {
                matchingPaths.push_back("cd " + pathPart);
                continue;
            }

            matchingPaths.push_back("cd " + basePath + pathPart);
        }
    }

    if (matchingPaths.size() == 0)
    {
        return std::vector<std::string>();
    }

    return matchingPaths;
}

void printCdPaths(const std::vector<std::string> &paths)
{
    if (paths.size() > 100)
    {
        std::cout << '\n'
                  << "Display all " << paths.size() << " possibilities? (y/n)";

        char response;

        std::cin >> response;

        if (response != 'y')
        {
            std::cout << "\n\n";
            return;
        }
    }

    std::size_t maxLength = 0;
    for (const auto &path : paths)
    {
        if (path.length() > maxLength)
        {
            maxLength = path.length();
        }
    }

    const int COLUMN_WIDTH = maxLength + 2;
    const int PATHS_PER_LINE = 5;
    int count = 0;

    std::cout << "\n";

    bool isNewLine = false;
    for (const auto &path : paths)
    {
        std::string toPrint = path;

        std::size_t lastSlashPos = path.find_last_of('/');
        if (lastSlashPos != std::string::npos)
        {
            if (lastSlashPos == path.length() - 1)
            {
                lastSlashPos = path.find_last_of('/', lastSlashPos - 1);
            }

            toPrint = path.substr(lastSlashPos + 1);
        }

        if (toPrint.find("cd ") == 0)
        {
            toPrint = toPrint.substr(3);
        }

        std::cout << std::left << std::setw(COLUMN_WIDTH) << toPrint;

        count++;

        if (count % PATHS_PER_LINE == 0)
        {
            std::cout << '\n';
            isNewLine = true;
        }
        else
        {
            isNewLine = false;
        }
    }

    std::cout << "\n\n";
}
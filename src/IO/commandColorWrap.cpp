#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

const std::string YELLOW_COLOR = "\033[1;33m";
const std::string RESET_COLOR = "\033[0m";

std::string sanitizeCommandFromColor(std::string buffer, int *cursorPosition)
{
    // Sanitize YELLOW_COLOR
    std::size_t foundPosition = 0;
    while ((foundPosition = buffer.find(YELLOW_COLOR, foundPosition)) != std::string::npos)
    {
        if (foundPosition < static_cast<std::size_t>(*cursorPosition))
        {
            *cursorPosition -= YELLOW_COLOR.size();
        }
        buffer.erase(foundPosition, YELLOW_COLOR.size());
    }

    // Sanitize RESET_COLOR
    foundPosition = 0;
    while ((foundPosition = buffer.find(RESET_COLOR, foundPosition)) != std::string::npos)
    {
        if (foundPosition < static_cast<std::size_t>(*cursorPosition))
        {
            *cursorPosition -= RESET_COLOR.size();
        }
        buffer.erase(foundPosition, RESET_COLOR.size());
    }

    return buffer;
}

std::string balanceCommandColor(std::string buffer, int *cursorPosition)
{
    int colorCodeBalance = 0;
    std::size_t lastYellowColorPosition = std::string::npos;

    for (std::size_t i = 0; i < buffer.size();)
    {
        if (buffer[i] == '\033')
        {
            if (i + YELLOW_COLOR.size() <= buffer.size() && buffer.substr(i, YELLOW_COLOR.size()) == YELLOW_COLOR)
            {
                colorCodeBalance++;
                lastYellowColorPosition = i;
                i += YELLOW_COLOR.size();
            }
            else if (i + RESET_COLOR.size() <= buffer.size() && buffer.substr(i, RESET_COLOR.size()) == RESET_COLOR)
            {
                colorCodeBalance--;
                i += RESET_COLOR.size();
            }
            else
            {
                i++;
            }
        }
        else
        {
            i++;
        }
    }

    if (colorCodeBalance > 0 && lastYellowColorPosition != std::string::npos)
    {
        buffer.erase(lastYellowColorPosition, YELLOW_COLOR.size());

        if (*cursorPosition > static_cast<int>(lastYellowColorPosition))
        {
            *cursorPosition -= YELLOW_COLOR.size();
        }
    }

    return buffer;
}

std::string wrapCommandIntoColor(std::string buffer, int *cursorPosition)
{
    // Get all the system commands
    std::vector<std::string> systemCommands;

    char *pathEnv = std::getenv("PATH");
    if (pathEnv == nullptr)
    {
        return buffer;
    }

    std::string pathEnvStr = pathEnv;

    std::size_t position = 0;
    while ((position = pathEnvStr.find(':')) != std::string::npos)
    {
        std::string directory = pathEnvStr.substr(0, position);
        pathEnvStr.erase(0, position + 1);

        try
        {
            for (const auto &entry : std::filesystem::directory_iterator(directory))
            {
                if (std::filesystem::is_regular_file(entry) && (entry.status().permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
                {
                    std::string command = entry.path().filename().string();
                    systemCommands.push_back(command);
                }
            }
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Error accessing directory: " << e.what() << std::endl;
        }
    }

    try
    {
        for (const auto &entry : std::filesystem::directory_iterator(pathEnvStr))
        {
            if (std::filesystem::is_regular_file(entry) && (entry.status().permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
            {
                std::string command = entry.path().filename().string();
                systemCommands.push_back(command);
            }
        }
    }
    catch (const std::filesystem::filesystem_error &e)
    {
        std::cerr << "Error accessing last directory in PATH: " << e.what() << std::endl;
    }

    std::sort(systemCommands.begin(), systemCommands.end());

    std::string wrappedBuffer = buffer;
    int cursorOffset = 0;

    for (const auto &command : systemCommands)
    {
        std::size_t foundPosition = 0;

        while ((foundPosition = wrappedBuffer.find(command, foundPosition)) != std::string::npos)
        {
            bool isWordBoundaryBefore = (foundPosition == 0) || std::isspace(wrappedBuffer[foundPosition - 1]);

            bool isWordBoundaryAfter = (foundPosition + command.size() < wrappedBuffer.size()) &&
                                       std::isspace(wrappedBuffer[foundPosition + command.size()]);

            if (isWordBoundaryBefore && isWordBoundaryAfter)
            {
                if (foundPosition <= *cursorPosition)
                {
                    *cursorPosition += YELLOW_COLOR.length();
                }

                wrappedBuffer.insert(foundPosition, YELLOW_COLOR);
                foundPosition += YELLOW_COLOR.length();

                foundPosition += command.length();

                if (foundPosition <= *cursorPosition)
                {
                    *cursorPosition += RESET_COLOR.length();
                }

                wrappedBuffer.insert(foundPosition, RESET_COLOR);
                foundPosition += RESET_COLOR.length();
            }
            else
            {
                foundPosition += command.length();
            }
        }
    }

    return wrappedBuffer;
}
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <string>
#include <termios.h>
#include <regex>
#include "IO.hpp"
#include "../history/commandHistory.hpp"
#include "../shell.hpp"
#include "../prompt/prompt.hpp"

std::string tempCommandBuffer;
bool isStoringCommand = false;

void enableRawMode(struct termios &orig_termios)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disableRawMode(struct termios &orig_termios)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

bool isAtAnsiEscapeSequenceBackwards(const std::string &inputBuffer, std::size_t cursorPosition)
{
    if (cursorPosition == 0)
    {
        return false;
    }

    if (inputBuffer[cursorPosition - 1] == 'm')
    {
        std::size_t start = cursorPosition - 2;

        while (start > 0 && inputBuffer[start] != '\033')
        {
            start--;
        }

        return start >= 0 && inputBuffer[start] == '\033' && inputBuffer[start + 1] == '[';
    }

    return false;
}

bool isAtAnsiEscapeSequenceForwards(const std::string &inputBuffer, std::size_t cursorPosition)
{
    // Check if position is within bounds
    if (cursorPosition >= inputBuffer.size())
    {
        return false;
    }

    // Check if the current character starts an ANSI escape sequence
    return inputBuffer[cursorPosition] == '\033' && 
           cursorPosition + 1 < inputBuffer.size() && 
           inputBuffer[cursorPosition + 1] == '[';
}

std::size_t getAnsiEscapeSequenceLengthBackwards(const std::string &inputBuffer, std::size_t cursorPosition)
{
    if (cursorPosition < 2 || !isAtAnsiEscapeSequenceBackwards(inputBuffer, cursorPosition))
    {
        return 0;
    }

    std::size_t start = cursorPosition - 2;
    std::size_t end = cursorPosition;

    while (start > 0 && inputBuffer[start - 1] != '\033')
    {
        start--;
    }

    return end - start + 1;
}

std::size_t getAnsiEscapeSequenceLengthForwards(const std::string &buffer, std::size_t position)
{
    if (position >= buffer.size() || !isAtAnsiEscapeSequenceForwards(buffer, position))
    {
        return 0;
    }

    std::size_t end = position + 2; 

    while (end < buffer.size() && buffer[end] != 'm')
    {
        end++;
    }

    return (end < buffer.size() && buffer[end] == 'm') ? (end - position + 1) : 0;
}

void refreshLine(const char *prompt, std::string inputBuffer, int cursorPosition)
{
    std::cout << "\r\033[K"; // Clear the current line
    std::cout << prompt << inputBuffer; 

    int len = inputBuffer.length();
    int adjustedPosition = cursorPosition;

    for (int i = cursorPosition; i < len; ++i)
    {
        if (inputBuffer[i] == '\033')
        {
            std::size_t ansiLength = getAnsiEscapeSequenceLengthForwards(inputBuffer, i);
            if (ansiLength > 0)
            {
                adjustedPosition += ansiLength; 
                i += ansiLength - 1;
            }
        }
    }

    if (adjustedPosition < len)
    {
        for (int i = len; i > adjustedPosition; --i)
        {
            std::cout << "\b";
        }
    }
}

std::string handleEscChars(std::string inputBuffer, int *cursorPosition, int *historyPosition)
{
    char seq[3];
    seq[0] = getchar();
    seq[1] = getchar();

    Prompt &prompt = Prompt::getInstance();

    if (seq[0] != '[')
    {
        return inputBuffer;
    }

    if (seq[1] == 'A') // Up arrow
    {
        if (globalCommandHistory.size() == *historyPosition)
        {
            return inputBuffer;
        }

        *historyPosition = *historyPosition + 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return inputBuffer;
        }

        if (!isStoringCommand)
        {
            tempCommandBuffer = inputBuffer;
            //std::cout << tempCommand << std::endl;
            isStoringCommand = true;
        }

        std::cout << "\r\033[K"; // Clear the current line
        prompt.printPrompt();

        std::cout << lastCommand;

        inputBuffer = lastCommand;

        *cursorPosition = inputBuffer.size();

        return inputBuffer;
    }

    if (seq[1] == 'B') // Down arrow
    {
        if (*historyPosition == 1)
        {
            std::cout << "\r\033[K"; // Clear the current line
            std::cout << prompt.getPrompt() << tempCommandBuffer;
            *cursorPosition = tempCommandBuffer.size();

            *historyPosition = *historyPosition - 1;

            isStoringCommand = false;
            return tempCommandBuffer;
        }

        *historyPosition = *historyPosition - 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return inputBuffer;
        }

        std::cout << "\r\033[K"; // Clear the current line
        prompt.printPrompt();

        std::cout << lastCommand;

        inputBuffer = lastCommand;

        *cursorPosition = inputBuffer.size();

        return inputBuffer;
    }

    if (seq[1] == 'D') //* Left arrow
    {
        if (*cursorPosition > 0)
        {
            // Move cursor back by one character first
            std::size_t currentPos = *cursorPosition - 1; // Check the position before decrementing

            // Keep track of the initial position
            std::size_t initialPos = currentPos;

            while (currentPos > 0 && isAtAnsiEscapeSequenceBackwards(inputBuffer, currentPos))
            {
                std::size_t ansiLength = getAnsiEscapeSequenceLengthBackwards(inputBuffer, currentPos);
                currentPos -= ansiLength; // Move cursor to the left of the escape sequence
            }

            // Update the position after skipping the ANSI escape sequence
            *cursorPosition = currentPos;

            // Ensure position doesn't go negative
            if (*cursorPosition < 0)
                *cursorPosition = 0;

            // Output the left arrow movement in the terminal
            std::cout << "\033[D"; // Move cursor left visually
        }
    }

    if (seq[1] == 'C') // Right arrow
    {
        if (*cursorPosition < inputBuffer.size())
        {
            // Check if we're currently at an ANSI escape sequence
            if (isAtAnsiEscapeSequenceForwards(inputBuffer, *cursorPosition))
            {
                // Skip over the entire escape sequence
                std::size_t ansiLength = getAnsiEscapeSequenceLengthForwards(inputBuffer, *cursorPosition);
                *cursorPosition += ansiLength; // Move cursor to the end of the escape sequence
            }
            else
            {
                // Move cursor forward by one character
                (*cursorPosition)++;
            }

            // Ensure position doesn't exceed buffer size
            if (*cursorPosition > inputBuffer.size())
                *cursorPosition = inputBuffer.size();

            // Output the right arrow movement in the terminal
            std::cout << "\033[C"; // Move cursor right visually
        }
    }

    if (seq[1] == '1') //* Home key
    {
        char t = getchar();

        if (t == '~')
        {
            *cursorPosition = 0;

            std::cout << "\r\033[K"; // Clear the current line
            prompt.printPrompt();
            std::cout << inputBuffer;

            for (int i = 0; i < inputBuffer.size(); i++)
            {
                std::cout << '\b';
            }
        }
    }

    if (seq[1] == '4') //* End key
    {
        char t = getchar();

        if (t == '~')
        {
            *cursorPosition = inputBuffer.size();

            std::cout << "\r\033[K"; // Clear the current line
            prompt.printPrompt();
            std::cout << inputBuffer;
        }
    }

    return inputBuffer;
}

std::string readLine()
{
    std::string inputBuffer;
    int inputCharacter;
    struct termios orig_termios;
    int cursorPosition = 0;
    int historyPosition = 0;

    enableRawMode(orig_termios);

    Prompt &promptInstance = Prompt::getInstance();

    std::string prompt = promptInstance.getPrompt();
    std::cout << prompt;

    while (true)
    {
        inputCharacter = getchar();

        if (inputCharacter == EOF || inputCharacter == '\n') //* Check for EOF or Enter key
        {
            if (inputBuffer.size() == 0)
            {
                std::cout << '\n';
            }

            std::cout << '\n';

            disableRawMode(orig_termios);

            if (inputBuffer.empty())
            {
                return std::string();
            }

            char *command = strdup(inputBuffer.c_str());
            globalCommandHistory.addCommand(command);

            historyPosition = 0;
            inputBuffer = sanitizeCommandFromColor(inputBuffer, &cursorPosition);

            return inputBuffer;
        }
        else if (inputCharacter == '\033') //* ESC key
        {
            inputBuffer = handleEscChars(inputBuffer, &cursorPosition, &historyPosition);
        }
        else if (inputCharacter == 127) //* Backspace key
        {
            if (!inputBuffer.empty() && cursorPosition > 0)
            {
                if (cursorPosition >= 2 && inputBuffer[cursorPosition - 1] == 'm')
                {
                    std::size_t startPosition = inputBuffer.rfind("\033[", cursorPosition - 1);
                    if (startPosition != std::string::npos && cursorPosition - startPosition <= 7)
                    {
                        if (startPosition > 0)
                        {
                            inputBuffer.erase(startPosition - 1, (cursorPosition - startPosition) + 1);
                            cursorPosition = startPosition - 1;
                        }
                    }
                    else
                    {
                        inputBuffer.erase(cursorPosition - 1, 1);
                        cursorPosition--;
                    }
                }
                else
                {
                    inputBuffer.erase(cursorPosition - 1, 1);
                    cursorPosition--;
                }

                inputBuffer = sanitizeCommandFromColor(inputBuffer, &cursorPosition);
                inputBuffer = wrapCommandIntoColor(inputBuffer, &cursorPosition);

                refreshLine(prompt.c_str(), inputBuffer, cursorPosition);
            }
        }
        else if (inputCharacter == 9) //* Tab key
        {
            std::regex cdRegex(R"(cd\s+)");

            if (std::regex_search(inputBuffer, cdRegex))
            {
                int cdPos = inputBuffer.find("cd ");
                std::string bufferCopy = inputBuffer.substr(cdPos + inputBuffer.size() - cdPos);

                std::vector<std::string> paths = tabCdHandler(inputBuffer);

                if (paths.size() == 1)
                {
                    inputBuffer = paths[0];
                    cursorPosition = inputBuffer.size();

                    refreshLine(prompt.c_str(), inputBuffer, cursorPosition);
                }
                else
                {
                    printCdPaths(paths);

                    std::cout << prompt << inputBuffer;
                }
            }
            else
            {
                std::vector<std::string> commands = tabCommandHandler(inputBuffer, cursorPosition);

                if (commands.size() == 1)
                {
                    inputBuffer = commands[0];
                    cursorPosition = inputBuffer.size();

                    inputBuffer = sanitizeCommandFromColor(inputBuffer, &cursorPosition);
                    inputBuffer = wrapCommandIntoColor(inputBuffer, &cursorPosition);

                    refreshLine(prompt.c_str(), inputBuffer, cursorPosition);
                }
                else
                {
                    printCommands(commands);

                    std::cout << prompt << inputBuffer;
                }
            }
        }
        else
        {
            inputBuffer.insert(cursorPosition, 1, static_cast<char>(inputCharacter));
            cursorPosition++;

            inputBuffer = wrapCommandIntoColor(inputBuffer, &cursorPosition);

            refreshLine(prompt.c_str(), inputBuffer, cursorPosition);
        }
    }
}
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

bool isAtAnsiEscapeSequenceBackwards(const std::string &buffer, std::size_t position)
{
    if (position == 0)
    {
        return false;
    }

    if (buffer[position - 1] == 'm')
    {
        std::size_t start = position - 2;

        while (start > 0 && buffer[start] != '\033')
        {
            start--;
        }

        return start >= 0 && buffer[start] == '\033' && buffer[start + 1] == '[';
    }

    return false;
}

bool isAtAnsiEscapeSequenceForwards(const std::string &buffer, std::size_t position)
{
    // Check if position is within bounds
    if (position >= buffer.size())
    {
        return false;
    }

    // Check if the current character starts an ANSI escape sequence
    return buffer[position] == '\033' && 
           position + 1 < buffer.size() && 
           buffer[position + 1] == '[';
}

std::size_t getAnsiEscapeSequenceLengthBackwards(const std::string &buffer, std::size_t position)
{
    if (position < 2 || !isAtAnsiEscapeSequenceBackwards(buffer, position))
    {
        return 0;
    }

    std::size_t start = position - 2;
    std::size_t end = position;

    while (start > 0 && buffer[start - 1] != '\033')
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

void refreshLine(const char *prompt, std::string buffer, int position)
{
    std::cout << "\r\033[K"; // Clear the current line
    std::cout << prompt << buffer; 

    int len = buffer.length();
    int adjustedPosition = position;

    for (int i = position; i < len; ++i)
    {
        if (buffer[i] == '\033')
        {
            std::size_t ansiLength = getAnsiEscapeSequenceLengthForwards(buffer, i);
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

std::string handleEscChars(std::string buffer, int *position, int *historyPosition)
{
    char seq[3];
    seq[0] = getchar();
    seq[1] = getchar();

    Prompt &prompt = Prompt::getInstance();

    if (seq[0] != '[')
    {
        return buffer;
    }

    if (seq[1] == 'A') // Up arrow
    {
        if (globalCommandHistory.size() == *historyPosition)
        {
            return buffer;
        }

        *historyPosition = *historyPosition + 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return buffer;
        }

        std::cout << "\r\033[K"; // Clear the current line
        prompt.printPrompt();

        std::cout << lastCommand;

        buffer = lastCommand;

        *position = buffer.size();

        return buffer;
    }

    if (seq[1] == 'B') // Down arrow
    {
        if (*historyPosition == 1) //? This should not clear the last command but give me the buffer written before accessing the history
        {
            std::cout << "\r\033[K"; // Clear the current line
            prompt.printPrompt();
            *historyPosition = *historyPosition - 1;

            return buffer;
        }

        *historyPosition = *historyPosition - 1;

        const char *lastCommand = globalCommandHistory.getCommand(*historyPosition);

        if (!lastCommand)
        {
            return buffer;
        }

        std::cout << "\r\033[K"; // Clear the current line
        prompt.printPrompt();

        std::cout << lastCommand;

        buffer = lastCommand;

        *position = buffer.size();

        return buffer;
    }

    if (seq[1] == 'D') //* Left arrow
    {
        if (*position > 0)
        {
            // Move cursor back by one character first
            std::size_t currentPos = *position - 1; // Check the position before decrementing

            // Keep track of the initial position
            std::size_t initialPos = currentPos;

            while (currentPos > 0 && isAtAnsiEscapeSequenceBackwards(buffer, currentPos))
            {
                std::size_t ansiLength = getAnsiEscapeSequenceLengthBackwards(buffer, currentPos);
                currentPos -= ansiLength; // Move cursor to the left of the escape sequence
            }

            // Update the position after skipping the ANSI escape sequence
            *position = currentPos;

            // Ensure position doesn't go negative
            if (*position < 0)
                *position = 0;

            // Output the left arrow movement in the terminal
            std::cout << "\033[D"; // Move cursor left visually
        }
    }

    if (seq[1] == 'C') // Right arrow
    {
        if (*position < buffer.size())
        {
            // Check if we're currently at an ANSI escape sequence
            if (isAtAnsiEscapeSequenceForwards(buffer, *position))
            {
                // Skip over the entire escape sequence
                std::size_t ansiLength = getAnsiEscapeSequenceLengthForwards(buffer, *position);
                *position += ansiLength; // Move cursor to the end of the escape sequence
            }
            else
            {
                // Move cursor forward by one character
                (*position)++;
            }

            // Ensure position doesn't exceed buffer size
            if (*position > buffer.size())
                *position = buffer.size();

            // Output the right arrow movement in the terminal
            std::cout << "\033[C"; // Move cursor right visually
        }
    }

    if (seq[1] == '1') //* Home key
    {
        char t = getchar();

        if (t == '~')
        {
            *position = 0;

            std::cout << "\r\033[K"; // Clear the current line
            prompt.printPrompt();
            std::cout << buffer;

            for (int i = 0; i < buffer.size(); i++)
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
            *position = buffer.size();

            std::cout << "\r\033[K"; // Clear the current line
            prompt.printPrompt();
            std::cout << buffer;
        }
    }

    return buffer;
}

std::string readLine()
{
    std::string buffer;
    int c;
    struct termios orig_termios;
    int position = 0;
    int historyPosition = 0;

    enableRawMode(orig_termios);

    Prompt &promptInstance = Prompt::getInstance();

    std::string prompt = promptInstance.getPrompt();
    std::cout << prompt;

    while (true)
    {
        c = getchar();

        if (c == EOF || c == '\n') //* Check for EOF or Enter key
        {
            if (buffer.size() == 0)
            {
                std::cout << '\n';
            }

            std::cout << '\n';

            disableRawMode(orig_termios);

            if (buffer.empty())
            {
                return std::string();
            }

            char *command = strdup(buffer.c_str());
            globalCommandHistory.addCommand(command);

            historyPosition = 0;
            buffer = sanitizeCommandFromColor(buffer, &position);

            return buffer;
        }
        else if (c == '\033') //* ESC key
        {
            buffer = handleEscChars(buffer, &position, &historyPosition);
        }
        else if (c == 127) //* Backspace key
        {
            if (!buffer.empty() && position > 0)
            {
                if (position >= 2 && buffer[position - 1] == 'm')
                {
                    std::size_t startPosition = buffer.rfind("\033[", position - 1);
                    if (startPosition != std::string::npos && position - startPosition <= 7)
                    {
                        if (startPosition > 0)
                        {
                            buffer.erase(startPosition - 1, (position - startPosition) + 1);
                            position = startPosition - 1;
                        }
                    }
                    else
                    {
                        buffer.erase(position - 1, 1);
                        position--;
                    }
                }
                else
                {
                    buffer.erase(position - 1, 1);
                    position--;
                }

                buffer = sanitizeCommandFromColor(buffer, &position);
                buffer = wrapCommandIntoColor(buffer, &position);

                refreshLine(prompt.c_str(), buffer, position);
            }
        }
        else if (c == 9) //* Tab key
        {
            std::regex cdRegex(R"(cd\s+)");

            if (std::regex_search(buffer, cdRegex))
            {
                int cdPos = buffer.find("cd ");
                std::string bufferCopy = buffer.substr(cdPos + buffer.size() - cdPos);

                std::vector<std::string> paths = tabCdHandler(buffer);

                if (paths.size() == 1)
                {
                    buffer = paths[0];
                    position = buffer.size();

                    refreshLine(prompt.c_str(), buffer, position);
                }
                else
                {
                    printCdPaths(paths);

                    std::cout << prompt << buffer;
                }
            }
            else
            {
                std::vector<std::string> commands = tabCommandHandler(buffer, position);

                if (commands.size() == 1)
                {
                    buffer = commands[0];
                    position = buffer.size();

                    buffer = sanitizeCommandFromColor(buffer, &position);
                    buffer = wrapCommandIntoColor(buffer, &position);

                    refreshLine(prompt.c_str(), buffer, position);
                }
                else
                {
                    printCommands(commands);

                    std::cout << prompt << buffer;
                }
            }
        }
        else
        {
            buffer.insert(position, 1, static_cast<char>(c));
            position++;

            buffer = wrapCommandIntoColor(buffer, &position);

            refreshLine(prompt.c_str(), buffer, position);
        }
    }
}
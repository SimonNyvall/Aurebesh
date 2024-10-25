#include <string>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <time.h>

std::string getHomeDirecotry()
{
    const char *HOME = getenv("HOME");
    return HOME ? std::string(HOME) : std::string();
}

const std::string ERROR_DIR_PATH = getHomeDirecotry() + "/.local/share/aurebesh/"; 
const std::string ERROR_FILE_PATH = getHomeDirecotry() + "/.local/share/aurebesh/error.log"; 

bool doesErrorLogFileExist()
{
    return std::filesystem::exists(ERROR_FILE_PATH);
}

int createErrorLogPath()
{
    if (!std::filesystem::exists(ERROR_DIR_PATH))
    {
        std::filesystem::create_directory(ERROR_DIR_PATH);
    }

    if (!std::filesystem::exists(ERROR_FILE_PATH))
    {
        std::ofstream outFile(ERROR_FILE_PATH);

        if (!outFile)
        {
            std::cerr << "Error creating error log file";
            return 1;
        }

        outFile.close();
    }

    return 0;
}

void logErrorToFile(std::string location, std::string logLevel, std::string message)
{
    time_t now = time(0);
    struct tm tstruct;
    char currentDateTime[80];
    tstruct = *localtime(&now);
    strftime(currentDateTime, sizeof(currentDateTime), "%Y-%m-%d.%X", &tstruct);

    std::string currentLogLineStr = currentDateTime;
    std::string logLine = currentLogLineStr + " - " + location + " - " + logLevel + " - " + message;

    std::ofstream outFile(ERROR_FILE_PATH, std::ios::app);
    outFile << logLine << std::endl;
    outFile.close();
}
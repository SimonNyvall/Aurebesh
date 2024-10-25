#ifndef ERROR_LOG_H
#define ERROR_LOG_H

#include <string>

bool doesErrorLogFileExist();
int createErrorLogPath();
void logErrorToFile(std::string location, std::string logLevel, std::string message);

#endif
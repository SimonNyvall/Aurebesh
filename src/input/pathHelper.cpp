#include <filesystem>
#include <unistd.h>
#include <string>
#include <algorithm>

bool isExecutable(const std::filesystem::path &path)
{
    return std::filesystem::is_regular_file(path) && access(path.c_str(), X_OK) == 0;
}

std::string toLower(const std::string &str)
{
    std::string lowerStr = str;

    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c)
                   { return std::tolower(c); });

    return lowerStr;
}
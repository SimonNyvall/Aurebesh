#include <filesystem>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <vector>

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

std::string findComonPrefix(const std::vector<std::string>& strings)
{
    if (strings.empty())
    {
        return {};
    }

    std::string prefix = strings[0];

    for (std::size_t i = 1; i < strings.size(); i++)
    {
        std::size_t j = 0;

        while (j < prefix.size() && j < strings[i].size() && prefix[j] == strings[i][j])
        {
            j++;
        }

        prefix = prefix.substr(0, j);

        if (prefix.empty())
        {
            break;
        }
    }

    return prefix;
}
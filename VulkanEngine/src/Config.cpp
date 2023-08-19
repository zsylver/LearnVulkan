#include "Config.h"

std::vector<std::string> Split(std::string line, std::string delimiter)
{
    std::vector<std::string> splitLine;

    size_t pos = 0;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos)
    {
        token = line.substr(0, pos);
        splitLine.push_back(token);
        line.erase(0, pos + delimiter.length());
    }
    splitLine.push_back(line);

    return splitLine;
}

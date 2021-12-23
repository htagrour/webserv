#include "function_utils.hpp"

std::string         handleSpace(const std::string & str)
{
    std::string     urlReplaced;
    std::string     workingStr;
    size_t          reachEnd;
    size_t          position;

    urlReplaced = "";
    workingStr = str;
    reachEnd = 0;
    while (true)
    {
        position = workingStr.find("%20");
        urlReplaced += str.substr(reachEnd, position);
        if (position == std::string::npos)
            break;
        urlReplaced += ' ';
        workingStr = workingStr.substr(position + 3);
        reachEnd += position + 3;
    }
    return (urlReplaced);
}
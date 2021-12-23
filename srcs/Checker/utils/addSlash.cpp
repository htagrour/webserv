

#include "utils.hpp"

std::string         ft::addSlash(const std::string & locationStr)
{
    std::string     newLocationStr;

    newLocationStr = locationStr;
    if (*locationStr.rbegin() != '/')
        newLocationStr += "/";
    return (newLocationStr);
}
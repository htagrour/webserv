

#include  "utils.hpp"


std::string             ft::removeSlash(const std::string & path)
{
    if (*path.rbegin() == '/')
        return (path.substr(0, path.length() - 1));
    return (path);
}
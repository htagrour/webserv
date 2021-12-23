

#ifndef UTILS_CHECKER_HPP
# define UTILS_CHECKER_HPP
#include <string>
#include <vector>
namespace ft
{
    std::string         addSlash(const std::string & locationStr);
    std::string         removeSlash(const std::string & path);
    std::string         SlashBeforeConcat(const std::string & s);
};
#endif
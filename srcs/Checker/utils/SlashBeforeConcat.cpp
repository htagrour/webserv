

#include "utils.hpp"


std::string     ft::SlashBeforeConcat(const std::string & s)
{
    if (!s.length() || s[0] == '/')
        return ("");
    return ("/");
}
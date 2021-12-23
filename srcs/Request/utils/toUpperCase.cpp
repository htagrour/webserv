#include "function_utils.hpp"

std::string     toUpperCase(const std::string & str)
{
    std::string     s("");


    for (size_t iter = 0; iter < str.length(); iter++)
    {
        s += toupper(str[iter]);
    }
    return (s);
}


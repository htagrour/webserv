#include "function_utils.hpp"


std::string      randomString(size_t length)
{
    const std::string lwords = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789$_";
    std::string generatedString;

    generatedString = "";
    std::srand(std::time(NULL));
    while (length > 0)
    {
        generatedString += lwords[rand() % lwords.length()];
        length--;
    }
    return (generatedString);
}
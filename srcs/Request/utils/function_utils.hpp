#ifndef FUNCTION_UTILS_HPP
# define FUNCTION_UTILS_HPP
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>

std::string     toUpperCase(const std::string & str);
std::string     randomString(size_t length);
std::string     handleSpace(const std::string & str);
#endif
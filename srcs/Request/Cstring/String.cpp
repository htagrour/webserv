#include "String.hpp"

size_t String::pos = -1;
//CONSTRUCTORS

String::String()
{
    _len = 0;
    buffer = nullptr;
}

String::String(const char *str, size_t bufferSize)
{
    _len = bufferSize;
    buffer = new char[_len + 1];
    std::memcpy(buffer, str, sizeof(char) * bufferSize);
}
String::String(size_t len, char c)
{
    _len = len;
    buffer = new char[len + 1];
    std::memset(buffer, c, sizeof(char) * (_len + 1));    
}

String::String(const String & str)
{
    _len = 0;
    buffer = nullptr;
    *this = str;
}

//OPERATOR OVERLOAD ASSIGNATION

String & String::operator=(const char *s)
{
    clearBuffer();
    _len = std::strlen(s);
    buffer = new char[_len + 1];
    std::memcpy(buffer, s, _len * sizeof(char));
    return (*this);
}

String & String::operator=(const String & str)
{
    if (this != &str)
    {
        clearBuffer();
        _len = str.length();
        buffer = new char[_len + 1];
        std::memcpy(buffer, str.base(), _len * sizeof(char));
    }
    return (*this);
}

//DESTRUCTOR
String::~String()
{
    clearBuffer();
}

// GETTERS
size_t      String::length() const
{
    return (_len);
}

const char *  String::base() const
{
    return (buffer);
}

//PRIVATE METHODS

void        String::clearBuffer()
{
    delete[] buffer;
    _len = 0;
    buffer = nullptr;
}

//OUTSIDE FUNCTION
std::ostream & operator<<(std::ostream & out, const String &s)
{
    out << s.base();
    return (out);
}

bool operator==(const String & lhs, const String & rhs)
{
    return (!std::memcmp(lhs.base(), rhs.base(), std::max(lhs.length(), rhs.length())));
}

// CONVERTER TO STD::STRING
String::operator std::string() const
{
    return (std::string(this->base()));
}

String::operator const char *() const
{
    return this->base();    
}
//OUT OF RANGE EXCEPTION 

String::OutOfRange::OutOfRange(const char *m) : message(m)
{
}

const char * String::OutOfRange::what() const throw()
{
    return (message);
}
//ACCESSOR
char & String::operator[](size_t index) throw(OutOfRange)
{
    if (index >= _len)
        throw OutOfRange("Index Out Of Range");
    return (buffer[index]);
}


String  String::substr(size_t start, size_t end) const
{
    size_t      iter;

    if (end == String::pos)
        end = _len;
    String s((start < end) ? end - start: 0, 0);
    iter = 0;
    while ((start + iter) < end)
    {
        s[iter] = base()[start + iter];
        iter++;
    }
    return (s);
}

size_t  String::findPosition(char c) const
{
    size_t position  = strchr(base(), c) - base();
    return ((position >= _len) ? pos : position);
}

size_t  String::findPosition(const char *s) const
{
    size_t position = strstr(base(), s) - base();
    return ((position >= _len) ? pos : position);
}

size_t  String::findPosition(const String & str) const
{
    size_t position = strstr(base(), str.base()) - base();
    return ((position > _len) ? pos : position);
}

String  &    String::operator+=(const String & str)
{
    String  s(str.length() + length() + 1, 0);
    if (length())
        std::memcpy(s.buffer, this->base(), length() * sizeof(char));
    if (str.length())
        std::memcpy(s.buffer + length(), str.base(), sizeof(char) * str.length());
    *this = s;
    return (*this);
}
/*
String  &    String::operator+=(const char * str)
{
    String  s(strlen(str) + length(), 0);
    if (length())
        strcpy(s.buffer, this->base());
    strcpy(s.buffer + this->length(), str);
    *this = s;
    return (*this);
}
*/
String  & String::operator+=(size_t position)
{
    size_t  iter;

    iter = 0;
    if (position > length())
        position = length();
    while (iter + position <= length())
    {
        buffer[iter] = buffer[position + iter];
        iter++;
    }
    while (iter < length())
    {
        buffer[iter] = 0;
        iter++;
    }
    _len = length() - position;
    return (*this);
}


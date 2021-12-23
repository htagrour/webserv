#ifndef STRING_HPP
#define STRING_HPP
#include <iostream>
#include <cstring>
#include <string>
#include <exception>


class String
{
    public:
        static size_t       pos;
        class   OutOfRange : public std::exception
        {
            private:
                const char *message;
            public:
                OutOfRange(const char *);
                const char *what() const throw();
        };
        //CONSTRUCTOR
        String();
        String(const char *str, size_t bufferSize);
        String(const String & str);
        String(size_t size, char value);
        //OPERTOR ASSIGNATION
        String & operator=(const String & str);
        String & operator=(const char *s);
        //DESTRACTOR
        ~String();
        //CONVERT FROM String 2 std::string
        operator std::string() const;
        operator const char *() const;
        //GET LENGTH OF String
        size_t  length() const;
        //GET String's buffer
        const char *base() const;
        //ACCESSOR
        char  & operator[](size_t index) throw(OutOfRange);
        // SUBSTR
        String       substr(size_t start, size_t end=String::pos) const;
        // FIND
        size_t       findPosition(char c) const;
        size_t       findPosition(const char *s) const;
        size_t       findPosition(const String & s) const;
        // COncatenate two strings
        String     & operator+=(const String & s);
        //String     & operator+=(const char *s);
        String     & operator+=(size_t position);
        private:
        size_t      _len;
        char        *buffer;
        void        clearBuffer();
};

std::ostream & operator<<(std::ostream & out, const String &s);
bool           operator==(const String &lhs , const String & rhs);
#endif
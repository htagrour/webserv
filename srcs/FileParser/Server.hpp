
#pragma once

#include <string>
#include <map>
#include <vector>
#include "StatusCode.hpp"
#include <iostream>
#include <fstream>
#include <cctype>
#include <set>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>
#include <algorithm>

namespace ft{

class Server {
    private:
    int port;
    size_t max_body_size;    
    bool upload;
    bool autoindex;
    int socket;
    std::string root;
    std::string host;
    std::string server_name;
    std::string cgi;
    std::string uri;
    std::map<std::string, Server> location;
    std::map<int, std::string> error_page;
    std::vector<std::string> index;
    std::set<Method> methods;
    std::pair<HttpStatus::StatusCode, std::string> redirect;
public:
    Server();
    Server(const Server &);
    ~Server();
    Server &operator = (const Server &);
    int &getPort();
    size_t &getMaxBodySize();
    bool &getUpload();
    bool &getAutoIndex();
    std::string & getRoot();
    std::string &getHost();
    std::string &getServerName();
    std::string &getCgi();
    std::string &getUri();
    int &getSocket();
    std::map<std::string, Server>& getLocation();
    std::map<int, std::string>& getErrorPage();
    std::vector<std::string>& getIndex();
    std::set<Method>& getMethods();
    std::pair<HttpStatus::StatusCode, std::string> &getRedirect();
    friend std::ostream& operator<<(std::ostream& os, Server& s);
};

}
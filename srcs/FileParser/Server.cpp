#include "Server.hpp"
#include "StatusCode.hpp"

namespace ft{
Server::Server(){
    char *cwd = getcwd(NULL, 0);
    port = 80;
    host = "0.0.0.0";
    root = cwd;
    max_body_size = std::string::npos;
    redirect.first = HttpStatus::None;
    upload = false;
    autoindex = false;
    free(cwd);
}

Server & Server::operator=(const Server &server)
{
    port = server.port;
    host = server.host;
    server_name = server.server_name;
    error_page = server.error_page;
    max_body_size = server.max_body_size;
    location = server.location;
    methods = server.methods;
    root = server.root;
    redirect = server.redirect;
    index = server.index;
    cgi = server.cgi;
    upload = server.upload;
    uri = server.uri;
    socket = server.socket;
    autoindex = server.autoindex;
    return *this;
}

Server::Server(const Server &server){
    *this = server;
}

Server::~Server(){

}

int &Server::getPort(){
        return this->port;
 }
int &Server::getSocket() { return socket;}
size_t &Server::getMaxBodySize(){
        return this->max_body_size;
}

bool &Server::getUpload(){
        return this->upload;
}
std::string &Server::getRoot(){
        return this->root;
}
std::string &Server::getServerName() {
        return this->server_name;
}
std::string &Server::getHost() {
        return this->host;
}
std::string &Server::getCgi(){
        return this->cgi;
}

bool & Server::getAutoIndex()
{
        return autoindex;
}

std::string &Server::getUri(){
        return this->uri;
}
std::map<std::string, Server>& Server::getLocation(){
        return this->location;
}
std::map<int, std::string>& Server::getErrorPage(){
        return this->error_page;
}
std::vector<std::string>& Server::getIndex(){
        return this->index;
}
std::set<Method>& Server::getMethods(){
        return this->methods;
}
std::pair<HttpStatus::StatusCode, std::string> &Server::getRedirect(){
        return this->redirect;
}

std::ostream& operator<<(std::ostream& os, Server& s)
{
    os << "Server name :"<<s.server_name << std::endl;
    os << "Port        :"<<s.port << std::endl;
    os << "Root        :"<<s.root << std::endl;
    os << "Host        :"<<s.host << std::endl;
    os << "Cgi         :"<<s.cgi << std::endl;
    os << "Uri         :"<<s.uri << std::endl;
    os << "MaxBodySize :"<<s.max_body_size<< std::endl;
    os << "Autoindexx     :"<<s.autoindex << std::endl;
    os << "Upload      :"<<s.upload << std::endl;
os << "Autoindex : " << s.getAutoIndex() << std::endl;

    os << "Error Pages {" << std::endl;
        for (std::map<int, std::string>::iterator it = s.error_page.begin();
        it != s.error_page.end(); it++)
        {
                os << "      Error : " << it->first << "-" << it->second << std::endl;
        }
        os << "}" << std::endl;
        os << "Indexs {" << std::endl;
        for (std::vector<std::string>::iterator it = s.index.begin();
        it != s.index.end(); it++)
        {
                os << "      Index : " << *it  << std::endl;
        }
    os << "}" << std::endl;
        os << "Methods {" << std::endl;
        for (std::set<Method>::iterator it = s.methods.begin();
        it != s.methods.end(); it++)
        {
                os << "      Method : " << getMethodName(*it) << std::endl;

        }
    os << "}" << std::endl;
            os << "Redirect {" << std::endl;
                os << "      Http Status code : " << s.redirect.first <<" Path: "<< s.redirect.second  << std::endl;
    os << "}" << std::endl;
    std::cout << "-------------------------" << std::endl;
        os << "Locations {" << std::endl;
        for (std::map<std::string, Server>::iterator it = s.location.begin();
        it != s.location.end(); it++)
        {
                os << "      Location path : " << it->first  << std::endl;
                os << "      Autoindex : " << it->second.getAutoIndex() << std::endl;

                // os << "     *" << it->second. << std::endl;
        }
    os << "}" << std::endl;
    return os;
}
}
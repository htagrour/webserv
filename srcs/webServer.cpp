#include "HttpServer/HttpServer.hpp"

int main(int rc, char **argv)
{
    try
    {
        ft::HttpServer myServer;
        ft::ConfigFile file(rc, argv);
        
        
        signal(SIGPIPE, SIG_IGN);
        myServer.setUp(file.getServers());
        myServer.ShowMe();
        myServer.Run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
    return 0;
}
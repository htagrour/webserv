
#include "HttpServer.hpp"

ft::HttpServer::HttpServer()
{
    FD_ZERO(&writeFd);
    FD_ZERO(&readFd);
    maxFd = 0;
};

void ft::HttpServer::setUp(std::vector<Server> &servers) throw (SetUpFailedEx)
{
    for (std::vector<Server>::iterator it = servers.begin();
        it != servers.end(); it++)
        {
            if (setUpServer(*it)) // if somthing goes wrong
                throw(SetUpFailedEx());
        }
        // fun(this->servers);
}

// this Func check if there is a Server who has the same Port and Host
std::map<int , std::vector <ft::Server> >::iterator ft::HttpServer::checkPortAndHost(Server &server) 
{
    for (std::map<int , std::vector<Server> >::iterator it = servers.begin();\
         it != servers.end(); it++)
         {
             for (std::vector<Server>::iterator jt = it->second.begin();\
                    jt!= it->second.end(); jt++)
                    if (jt->getPort() == server.getPort() && jt->getHost() == server.getHost())
                        return it;
         }
    return servers.end();
}

int ft::HttpServer::setUpServer(Server &server)
{
    int newSocket;
    struct sockaddr_in address;

    std::map<int , std::vector <ft::Server> >::iterator ret = checkPortAndHost(server);
    if (ret != servers.end())
    {
        server.getSocket() = ret->first;
        ret->second.push_back(server);
        return 0;
    }
    if ((newSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) // create master socket 
    {
        std::cout << "Create master socket Failed!!" << std::endl;
        // exit(1);
        return 1;
    }
    // set masterSocket on non_blocking mode
    // fcntl(newSocket, F_SETFL, O_NONBLOCK);


    // initialize address structur
    bzero(&address, sizeof(address));
    address.sin_port = htons(server.getPort());
    address.sin_family = AF_INET; 
    inet_aton(server.getHost().c_str(), &address.sin_addr);
    // check if is a valid addr
    if (address.sin_addr.s_addr == INADDR_NONE)
    {
        std::cout << "BAD ADDRESS" << std::endl;
        // exit(1);
        close(newSocket);
        return 1;
    }

    // bind socket to address
    if (bind(newSocket, (struct sockaddr*)&address, sizeof(address)) == -1)
    {
        std::cout << "Failed to Bind " << newSocket << " socket" << std::endl;
        close(newSocket);
        return 1;
    }

    if (listen(newSocket, 1000) == -1)
    {
        std::cout << "Failed to listen on " << newSocket << " socket" << std::endl;
        close(newSocket);
        return 1;
    }
    
    if(addNewSocket(newSocket))
        return 1;
    server.getSocket() = newSocket;
    servers[newSocket].push_back(server);
    return 0;
}

int ft::HttpServer::addNewSocket(int newSocket)
{
    if (fcntl(newSocket, F_SETFL, O_NONBLOCK) == -1)
    {
        close(newSocket);
        std::cout << "Failed to Make A Socket in NonBlocking mode" << std::endl;
        return -1;
    }
    FD_SET(newSocket, &readFd);
    if(newSocket > maxFd)
        maxFd = newSocket;
    return 0;
}

void ft::HttpServer::Run()
{
    fd_set writeFdTmp;
    fd_set readFdTmp;

    while(1)
    {
        std::memcpy(&readFdTmp, &readFd, sizeof(readFd));
        std::memcpy(&writeFdTmp, &writeFd, sizeof(writeFd));

        if (select(maxFd + 1, &readFdTmp, &writeFdTmp, NULL, NULL) == -1)
        {
            std::cout << "FAILED TO SELECT A SOCKET" << std::endl;
            continue;
        }
        proccessConnections(readFdTmp, writeFdTmp);
        FD_ZERO(&readFdTmp);
        FD_ZERO(&writeFdTmp);
    }
}

void ft::HttpServer::proccessConnections(fd_set &readFdTmp, fd_set &writeFdTmp)
{

    for(int fd = 0; fd <= maxFd; fd++)
    {
        if (FD_ISSET(fd, &readFdTmp))
        {
            if (servers.count(fd)) // new connection
                newConnection(fd);
            else
                readFromSocket(fd);
        }
        if (FD_ISSET(fd, &writeFdTmp))
            writeToSocket(fd);
    }
}



void ft::HttpServer::ShowMe()
{
    std::cout << "Total Servers: " << servers.size() << std::endl;
    for (std::map<int ,std::vector<Server> >::iterator it = servers.begin(); it != servers.end();it++)
    {
        for (std::vector<Server>::iterator jt = it->second.begin(); jt != it->second.end(); jt++)
        {
            std::cout << "Server listen on " << jt->getHost() << ":" << jt->getPort();
            std::cout << " On masterSocket " << jt->getSocket();
            std::cout << " Name: " << jt->getServerName() << std::endl;
        }
    }
}

/*---------------------------------------------------*/

void ft::HttpServer::readFromSocket(int fd)
{
    char buffer[MAX_LINE];
    int ret;
    std::map<int, ft::Request>::iterator it;

    if ((ret = recv(fd, buffer, MAX_LINE, 0)) > 0)
    {
        Request &request = requests[fd];

        
        if (request.parseBuffer(buffer, ret))
        {
            std::cout << "Request has recived" << std::endl;
            FD_SET(fd, &writeFd); 
            FD_CLR(fd,&readFd);// dep
        }
    }
    else
    {
        if (ret == -1)
            std::cout << "Faild to Read from " << fd << " socket" << std::endl;
        else
        {
            std::cout << "Connection is closed" << std::endl;
        }
        deleteSocket(fd, 1);
    }

    bzero(&buffer, MAX_LINE);
}

void ft::HttpServer::writeToSocket(int fd)
{
    size_t ret;
    bool close;
    Response &res = responses[fd];
    Request & req = requests[fd];
    res.executeRequest(req);
    std::string toSend = res.getBuffer(TOSEND);
    ret = send(fd, toSend.c_str(), toSend.length(), 0);
    if (ret <= 0)
    {
        std::cout << "ERROR SENDING RES" << std::endl;
        deleteSocket(fd, 1);
        return;
    } 
    res.sendSuccessed(ret);
    if (res.isCompleted())
    {
        std::cout << "Response has sended" << std::endl;
        close = requests[fd].getAttributes().find("Connection")->second == " close";
        deleteSocket(fd, close); // depend
        if (!close)
            FD_SET(fd, &readFd);
    }
}

void ft::HttpServer::newConnection(int masterSocket)
{
    int newSocket;
    struct sockaddr_in address;
    int addrLen;

    if ((newSocket = accept(masterSocket, (struct sockaddr*)&address, (socklen_t*)&addrLen)) == -1)
    {
        std::cout << "Failed to accept New connection on "<< masterSocket << " socket" << std::endl;
        return; 
    }
    if (!addNewSocket(newSocket))
    {
        requests[newSocket].setServers(servers[masterSocket]);
        std::cout << "New Connection " << newSocket << std::endl;
    }
}

void ft::HttpServer::deleteSocket(int fd, int flag)
{

        std::cout << "Socket deleted" << std::endl;
        requests.erase(fd);
        responses.erase(fd);
        FD_CLR(fd, &writeFd);
        FD_CLR(fd, &readFd);
        if(flag)
            close(fd);
}

ft::HttpServer::~HttpServer()
{
   for (std::map<int, std::vector<Server> >::iterator it = servers.begin();\
        it != servers.end(); it++)
    {
        close(it->first);
    }
    FD_ZERO(&writeFd);
    FD_ZERO(&readFd);
}
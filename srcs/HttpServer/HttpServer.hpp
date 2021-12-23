
#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <vector>
#include <map>
#include <sys/types.h>
#include <exception>
#include "../Response/Response.hpp"
#include "../FileParser/ConfigFile.hpp"
#include "../Request/Request.hpp"
#include <signal.h>


#define MAX_LINE 1024 * 20
#define TOSEND 10000
namespace ft
{

	class HttpServer
	{

		public:

			HttpServer();
			HttpServer(const std::vector<Server> &servers);

			class SetUpFailedEx : public std::exception
			{
				const char *what() const throw()
				{
					return ("Error Occured While SetUping The Server");
				}
			};
			void setUp(std::vector<Server> &servers) throw (SetUpFailedEx );
			const std::vector<Server> 				&getServerBySocket(int socket);
			void ShowMe();
			void Run();
			~HttpServer();

		private:
			int setUpServer(Server &server);
			void proccessConnections(fd_set &readFdTmp, fd_set &writeFdTmp);
			void newConnection(int masterSocket);
			int  addNewSocket(int newSocket);
			void readFromSocket(int fd);
			void writeToSocket(int fd);
			void deleteSocket(int fd, int flag);
			std::map<int , std::vector <Server> >::iterator checkPortAndHost(Server &server);      
		private:

			// std::vector<Server> servers;
			std::map<int , Request> requests;
			std::map<int,  std::vector<Server> > servers;
			std::map<int , Response> responses;
			int maxFd;
			fd_set readFd;
			fd_set writeFd;

	};

}

#endif
#ifndef RESPONSE_HPP
#define RESPONSE_HPP
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdio>
#include <map>
#include "../FileParser/StatusCode.hpp"
#include "../Request/Request.hpp"
#include "../Checker/checker.hpp"
#include "../Cgi/cgi.hpp"
#include "utils/utils.hpp"
#include <unistd.h>
#include<dirent.h>

#define HTTP_VERSION "HTTP/1.1"
#define READ_BODY_SIZE 1024
enum PathStatus {
	NOTEX,
	FIL,
	DIRC
};

namespace ft
{
	class Response
	{
		
		typedef				void(Response::*methodF)(const Request&,Checker&);
		int										_status_code;
		std::string								_reason_phrase;
		std::string								_http_version;
		bool									_isCgi;
		std::string								_cgiPath;
		std::map<std::string, std::string>		_headers;
		std::map<Method, methodF>				_methods;
		std::ifstream							_body;
		std::stringstream						_buffer;
		off_t									_ContentLength;
		bool									_HeaderSend;
		bool									_complted;
		std::string								_bufferToSend;
		size_t									_bodyRead;
		bool 									_excuted;
		std::map<int, const char *>				_errorPages;
		
		public:
		class FileNotFound: public std::exception
		{
			public:
				const char * what() const throw();
		};
		class EndOfBuffer: public std::exception
		{
			public:
				const char * what() const throw();
		};
		Response(const Response & res);
		Response & operator=(const Response & res);

		void						getMethod(const Request& req, Checker & checker);
		void						postMethod(const Request& req, Checker & checker);
		void						deleteMethod(const Request& req, Checker & checker);
		void						unknownMethod(const Request& req, Checker & checker);
		PathStatus					isFile(const std::string &path);
		void 						genarateFolderList(const char *str, const Request& req);
		void						generateErrorPage(int statusCode);
		void						uploadFile(const Request& req, Checker &checker,const std::string &boundry);
		void						sendError(Checker &checker,int statusCode);
		std::string					readFromBodyFile(size_t size);
		public:
			size_t sended;
			Response();
			// explicit Response(const int status_code, const Request & req);
			~Response(); 
			std::string				getBuffer(size_t Size);
			void					executeRequest(Request &req);
			void					generateResponseBuffer(void);
			void					AddResponseBody(const char * file) throw(FileNotFound);
			//void					AddResponseBodyBuffer(const std::string &buffer);
			void					AddHeaders(const std::string & key, const std::string & value);
			bool					isCompleted(void) const;
			void					sendSuccessed(size_t length);
			void					setStatusCode(int statusCode);
			bool					getLineFromBody(std::string & str);
			void					setCgi();
	};
}
#endif
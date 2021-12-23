#ifndef REQUEST_HPP
# define REQUEST_HPP
# include <vector>
# include "Cstring/String.hpp"
# include <map>
# include <exception>
# include <sstream>
# include <iostream>
# include <utility>
# include <iterator>
# include "utils/function_utils.hpp"
# include <cstdio>
# include <fstream>
# include "../FileParser/StatusCode.hpp"
# include "../FileParser/Server.hpp"

namespace ft
{
	class Request
	{
		size_t                          		line;
		Method                          		method;
		std::string                    			http_version;
		std::string                    			url;
		size_t                          		contentLength;
		std::map<std::string, std::string>   	attributes;
		Server									_server;
		std::vector<Server>						_all_servers;
		String                              	StoredBuffer;
		std::stringstream						StoredChunkedBuffer;
		bool                                 	empty_line;
		std::string                          	file;
		size_t                              	currentBodySize;
		bool                                	isFileCreated;
		std::ofstream                       	BodyFile;
		int                                 	listenSocket;
		bool                                	isCgi;
		size_t                              	_body_size_limit;
		bool                                	_body_reach_limit;
		size_t									reachedLength;
		size_t									currentChunkedLength;
		std::string                         	query;
		std::string                         	root;
		std::string                         	index;
		public:
			class FileNotExist: public std::exception
			{
				public:
					const char * what() const throw();
			};
			Request();
			~Request();
			bool                                    	parseBuffer(const char * buffer, size_t bufferSize);
			bool										ParseChunkedBody(String & buffer);
			Method                                  	getMethod() const;
			const std::map<std::string, std::string> 	& getAttributes(void) const;
			const std::string   &                   	getHttpVersion(void) const;
			const std::string   &                   	getUrl(void) const;
			void                                    	setCorrespondingServer();
			void										setServers(std::vector<Server> servers);
			size_t                                  	getContentLength() const;
			const   std::string  &                  	getFileName() const throw(FileNotExist);
			bool                                   		CheckBodyLimit() const;
			bool                                    	getIsCgi() const;
			int                                     	getListenSocket() const;
			const std::string							& getPath() const;
			const std::string                       	& getIndex() const;
			const std::string                       	& getQuery() const;
			Server										  getServer();
			bool										isBodyReachLimit() const;
			void 										setBodyLimit(size_t limit);
			void                                     	setCgi();
		private:
			int                                     	ParseFirstLineHeader(String & buffer);
			int                                    		EndOfLine(String & buffer);
			int                                     	EndOfFile(String & buffer);
			int                                     	ParseAttribute(String & buffer);
			void                                    	setMethod(const std::string &str);
			bool                                    	parseBody(String & buffer);
			bool                                    	createFile();
			void										findServerBasedOnHost(std::vector<Server> & servers, const std::string & host);
			void                                    	parseUrl();
	};
}
#endif

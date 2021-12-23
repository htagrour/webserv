

#ifndef CHECKER_HPP
#define CHECKER_HPP
#include <exception>
#include "../FileParser/ConfigFile.hpp"
#include "../FileParser/Server.hpp"
#include "../Request/Request.hpp"
#include "utils/utils.hpp"	
#include <sys/stat.h>
#include <iostream>

namespace   ft
{
	class  Checker
	{
		Server				_server;
		Server				_location;
		int					_status_code;
		std::string			_root;
		bool				_isCgi;
		bool				_is_auto_index;
		bool				_is_redirect;
		public:
			class UrlNotFound : public std::exception
			{
				public:
					const char *what() const throw();
			};
			Checker(Request &req);
			bool							isCgi() const;
			int								getStatusCode() const;
			const std::string				& getRoot() const;
			bool							isAutoIndex()  const;
			bool							checkRootExist() const;
			Server							getLocation();
			bool							isRedirect() const;
		private:
			void				findServerBasedOnHost(std::vector<Server> & servers, const std::string &string);
			void				findLocation(std::map<std::string, Server> & locations, const std::string & requestLocation) throw(UrlNotFound);
			bool				checkMethod(Method method, std::set<Method> & methods);
			bool 				CheckIndexExistance(const std::string & index);
			std::string			getValidIndex();
	};
	std::ostream 	& operator<<(std::ostream & os, const Checker & check);
};

#endif
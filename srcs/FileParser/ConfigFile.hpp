#pragma once

#include <string>
#include <map>
#include <vector>
#include "StatusCode.hpp"
#include <fstream>
#include <cctype>
#include <set>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>
#include <algorithm>
#include <iostream>     // std::cout, std::ostream, std::hex
#include <sstream>      // std::stringbuf
#include <string>
#include "Server.hpp"
#include <sys/stat.h>

namespace ft{
	enum ParseError{
	UNEXPECTED_SYMBOL, UNEXPECTED_EOF, UNKNOWN_DIRECTIVE, DIRECTIVE_NOT_ALLOWED,
	DIRECTIVE_HAS_NO_OPENING, DIRECTIVE_NOT_TERMINATED, INVALID_ARGUMENT, DUPLICATE_LISTEN,
	DUPLICATE_SERVER_NAME, INVALIDE_PORT, INVALID_PORT, INVALID_ERROR_CODE, INVALID_STATUS_CODE,
	INVALID_METHOD, DUPLICATE_METHOD, DUPLICATE_LOCATION, FILE_DOES_NOT_EXIST, PARSING_ERROR
	};
	const std::string Sdirectives[] = {"server", "location", "listen", "server_name", "autoindex"};//4
	const std::string Ldirectives[] = {"error_page", "max_body_size", "root", "index", "http_method", "autoindex"};//6
	const std::string Odirectives[] = {"redirect", "cgi_pass", "upload_pass"};//3

	class ConfigFile
	{
		private:
			char ** _argv;
			int _argc;
			std::vector<Server> g_servers;
			Server * curr_server;
			Server * curr_Location;
			bool duplicate_listen;

		public:
			ConfigFile(int argc, char **argv);
			~ConfigFile();
			std::vector<Server> &getServers();
			void    openConfigFile(int argc, char **argv);
			void    parse(std::ifstream &file);
			void conflictServerName();
			void checkError(std::vector<std::string>& dir);
			void    printDirective(std::vector<std::string> &directive);
			std::string nextToken(std::ifstream & file);
			std::vector<std::string> nextDirective(std::ifstream & file);
			bool isDelimiter(char c);
			bool isNumber(std::string &n);
			bool isDerictive(const std::string &dir);
			bool isServer(const std::string &dir);
			bool isLocation(const std::string &dir);
			void error(ParseError errn, const std::string &arg);
			bool isExist (const std::string& name);
			void	checkDuplicate(std::vector<std::string>& dir);
	};
}
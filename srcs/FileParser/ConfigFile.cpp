#include "ConfigFile.hpp"


namespace ft{

	ConfigFile::ConfigFile(int argc, char **argv){
		_argc = argc;
		_argv = argv;
		curr_server = NULL;
		curr_Location = NULL;
		duplicate_listen = false;
		openConfigFile(argc, argv);
	}
	ConfigFile::~ConfigFile(){
		g_servers.clear();
	}

	std::vector<Server> &ConfigFile::getServers(){
		return g_servers;
	}
bool ConfigFile::isExist (const std::string& name) {
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0); 
}
	void    ConfigFile::openConfigFile(int argc, char **argv){
		std::ifstream file;
		std::stringbuf buffer; 
		std::ostream os (&buffer);
		if (argc == 2){
			file.open(argv[1]);
			if (file.is_open()){
				parse(file);
				file.close();
				if (g_servers.empty()){
					os << argv[0] << ": there is no server!";
					throw std::invalid_argument(buffer.str());
				}
			}else {
				os << argv[0] << ": open():"; 
				throw std::invalid_argument(buffer.str());
			}
		}else {
			os << argv[0] << ": wrong number of arguments!";
			throw std::invalid_argument(buffer.str());
		}
	}

	void ConfigFile::parse(std::ifstream &file){
		std::vector<std::string> dir;

		while (!(dir = nextDirective(file)).empty())
		{
			checkError(dir);
			Server *server = curr_Location ? curr_Location : curr_server;

			if (dir.size() == 2){
				if (dir[0] == "server"){
					g_servers.push_back(Server());
					curr_server = &g_servers.back();
				}else{
					error(INVALID_ARGUMENT, dir[0]);
				}
			}else if (dir.size() == 4 && dir[0] == "listen"){
				if (dir[0] == "listen"){
					std::string host = dir[1] == "localhost" ? "127.0.0.1" : dir[1];
					if (duplicate_listen){
						if (server->getHost() == host && server->getPort() == std::atoi(dir[2].c_str())){
							error(DUPLICATE_LISTEN, host + ":" + dir[2]);
						}
						}else{
							server->getHost() = host;
							if (!isNumber(dir[2])){
								error(INVALID_PORT, dir[2]);
							}
							server->getPort() = std::atoi(dir[2].c_str());
							duplicate_listen = true;
						}
				}else{
					error(INVALID_ARGUMENT, dir[0]);
				}
		}else if (dir[0] == "autoindex" && dir.size() > 2){
			if (dir[1] == "on")
				server->getAutoIndex() = true;
			else if (dir[1] == "off")
			{
				server->getAutoIndex() = false;
			}
			else
				error(INVALID_ARGUMENT, dir[1]);
		}
		else if (dir[0] == "http_method" && dir.size() > 2){
			server->getMethods().clear();
			for (size_t i = 1; i < dir.size() - 1; i++)
			{
				Method method = getMethod(dir[i]);
				if (method == UNKNOWN){
					error(INVALID_METHOD, dir[i]);
				}else if (std::find(server->getMethods().begin(), server->getMethods().end(), method) != server->getMethods().end()){
					error(DUPLICATE_METHOD, dir[i]);
				}
				server->getMethods().insert(method);
			}
		}else if (dir[0] == "index" && dir.size() > 2){
			server->getIndex().clear();
			for (size_t i = 1; i < dir.size() - 1; i++){
				server->getIndex().push_back(dir[i]);
			}
		}else if (dir[0] == "error_page" && dir.size() > 3){
			for (size_t i = 1; i < dir.size() - 2; i++)
			{
				HttpStatus::StatusCode code = (HttpStatus::StatusCode) std::atoi(dir[i].c_str());
				if (!isNumber(dir[i]) || !isValidCode(code)){
					error(INVALID_ERROR_CODE, dir[i]);
				}
				if (!isExist(dir[dir.size() - 2])){
					error(FILE_DOES_NOT_EXIST, dir[dir.size() - 2]);
				}
				server->getErrorPage().insert(std::make_pair(code, dir[dir.size() - 2]));
			}
		}else if ((dir.size() == 3 || dir.size() == 4) && dir[0] == "redirect"){
			HttpStatus::StatusCode code = (HttpStatus::StatusCode) std::atoi(dir[1].c_str());
			if (!isNumber(dir[1]) || !isValidCode(code)){
				error(INVALID_ERROR_CODE, dir[1]);
			}
			server->getRedirect().first = code;
			server->getRedirect().second = dir.size() == 4 ? dir[2] : "";
		}else if (dir.size() == 3){
			if (dir[0] == "location"){
				if (curr_server->getLocation().find(dir[1]) == curr_server->getLocation().end()){
					curr_server->getLocation().insert(std::make_pair(dir[1], Server(*curr_server)));
					curr_Location = &curr_server->getLocation()[dir[1]];
					curr_Location->getUri() = dir[1];
				}else{
					error(DUPLICATE_LOCATION, dir[1]);
				}
			}else if (dir[0] == "server_name"){
				server->getServerName()= dir[1];
			}else if (dir[0] == "root"){
				server->getRoot() = dir[1];
			}else if (dir[0] == "max_body_size"){
				if (isNumber(dir[1]))
					server->getMaxBodySize() = std::atoi(dir[1].c_str());
				else 
					error(INVALID_ARGUMENT, dir[1]);
			}else if (dir[0] == "upload_pass"){
				server->getUpload ()= dir[1] == "on";
			}else if (dir[0] == "cgi_pass"){
				server->getCgi () = dir[1];
			}else{
				error(INVALID_ARGUMENT, dir[0]);
			}
		}else if (dir.size() == 1 && dir[0] == "}"){
			if (curr_Location){
				curr_Location = NULL;
			}else if (curr_server){
				conflictServerName();
				curr_server = NULL;
				duplicate_listen = false;
			}
		}else{
			error(INVALID_ARGUMENT, dir[0]);
		}
		}
		if (curr_server || curr_Location){
			std::cout << "frm here" << std::endl;
			error(UNEXPECTED_EOF, "}");
		}
	}

	void ConfigFile::conflictServerName(){
		if (curr_server){
			for (std::vector<Server>::iterator it = g_servers.begin(); &(*it) != curr_server; ++it)
			{
				if (it->getHost() == curr_server->getHost() && it->getPort() == curr_server->getPort() &&
					it->getServerName()== curr_server->getServerName()){
						std::cerr <<"webserv: conflicting server name '" << it->getServerName()<< "' on " << it->getHost() << ":" << it->getPort() << ", ignored" << std::endl;
						g_servers.pop_back();
					}
			}
			
		}
	}

	void	ConfigFile::checkDuplicate(std::vector<std::string>& dir){
		int dupl = 0;
		for (std::vector<std::string>::iterator iter = dir.begin(); iter != dir.end();iter++){

			if (isDerictive(*iter)){
				dupl++;
			}
		}
		if (dupl > 1){
			error(PARSING_ERROR, dir[0]);
		}
	}

	void ConfigFile::checkError(std::vector<std::string>& dir){
		if (isDelimiter(dir[0][0]) || dir.back() == "}"){
			if ((curr_server || curr_Location) && dir[0] == "}"){
				return;
			}
			std::cout << "f4484848." << std::endl;

			error(UNEXPECTED_SYMBOL, dir[0]);
		}else if (!isDerictive(dir[0])){
			std::cout << "error" << std::endl;
			error(UNKNOWN_DIRECTIVE, dir[0]);
		}else if ((curr_Location && !isLocation(dir[0])) || 
		(curr_server && !curr_Location && !isServer(dir[0])) || 
		(dir[0] != "server" && !curr_server)){
			error(DIRECTIVE_NOT_ALLOWED, dir[0]);
		}else if (!isDelimiter(dir.back()[0])){
			std::cout << "frm here...." << std::endl;

			error(UNEXPECTED_EOF, dir[0]);
		}else if ((dir[0] == "server" || dir[0] == "location") && dir.back() != "{"){
			error(DIRECTIVE_HAS_NO_OPENING, dir[0]);
		}else if ((dir[0] != "server" && dir[0] != "location" && dir.back() != ";")){
			error(DIRECTIVE_NOT_TERMINATED, dir[0]);
		}
	checkDuplicate(dir);
	}

	std::vector<std::string> ConfigFile::nextDirective(std::ifstream & file) {
		
		std::vector<std::string> directive;
		std::string token;

		while ((token = nextToken(file)) != "") {
			directive.push_back(token);
			if (token == "{" || token == "}" || token == ";") {
				break;
			}
		}
		return (directive);
	}

	std::string ConfigFile::nextToken(std::ifstream & file) {
		char c;
		bool isWord = false;
		std::string token;
		while ((c = file.get()) != EOF) {
			if (!isspace(c)) {
				token.push_back(c);
				isWord = true;
				if (isDelimiter(c)) {
					break;
				}
			} else if (isWord) {
				break;
			}

			if (isWord && (c = file.peek()) != EOF && isDelimiter(c)) {
				break;
			}
		}
		return token;
	}

	bool ConfigFile::isDerictive(const std::string &dir){
		return isServer(dir) || isLocation(dir);
	}

	bool ConfigFile::isNumber(std::string &n){
		for (size_t i = 0; i < n.length(); i++)
		{
			if(!std::isdigit(n[i])){
				return false;
			}
		}
		return true;
	}

	bool ConfigFile::isDelimiter(char c) {
		if (c == '{' || c == '}' || c == ';') {
			return true;
		}
		return false;
	}

	bool ConfigFile::isLocation(const std::string &dir){
		for (size_t i = 0; i < 6; i++)
		{
			if (dir == Ldirectives[i])
				return true;
		}
		for (size_t i = 0; i < 3; i++)
		{
			if (dir == Odirectives[i])
				return true;
		}
		return false;
	}

	bool ConfigFile::isServer(const std::string &dir){
		for (size_t i = 0; i < 5; i++)
		{
			if (dir == Sdirectives[i])
				return true;
		}
		for (size_t i = 0; i < 6; i++)
		{
			if (dir == Ldirectives[i])
				return true;
		}
		return false;
	}

	void ConfigFile::error(ParseError errn, const std::string &arg){
		std::stringbuf buffer; 
		std::ostream os (&buffer);
		if (errn == UNEXPECTED_EOF){
			os << "webserv: unexpected end of file, expecting '" << arg <<"'";
			throw std::invalid_argument(buffer.str());
		}else if (errn == UNEXPECTED_SYMBOL){
			os << "webserv: unexpected '" << arg << "'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == UNKNOWN_DIRECTIVE){
			os << "webserv: unknown directive '" << arg << "'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == DIRECTIVE_NOT_ALLOWED){
			os << "'" << arg << "' directive is not allowed here" ;
			throw std::invalid_argument(buffer.str()); 
		}else if (errn == DIRECTIVE_HAS_NO_OPENING){
			os << "webserv: directive '" << arg << "' has no opening '{'" ;
			throw std::invalid_argument(buffer.str()); 
		}else if (errn == DIRECTIVE_NOT_TERMINATED){
			os << "webserv: directive '" << arg << "' is not terminated by ';'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == INVALID_ARGUMENT){
			os << "webserv: invalid number of arguments in '" << arg << "' directive" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == INVALID_PORT){
			os << "webserv: invalid port '" << arg ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == INVALID_ERROR_CODE){
			os << "webserv: invalid error code '" << arg << "'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == INVALID_STATUS_CODE){
			os << "webserv: invalid status code '" << arg << "'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == INVALID_METHOD){
			os << "webserv: invalid method '" << arg << "'" ;
			throw std::invalid_argument(buffer.str());
		}else if (errn == DUPLICATE_METHOD){
			os << "webserv: deuplicated method '" << arg << "'" << std::endl;
			throw std::invalid_argument(buffer.str());
		}else if (errn == DUPLICATE_SERVER_NAME){
			os << "webserv: deuplicated server name '" << arg << "'" << std::endl;
			throw std::invalid_argument(buffer.str());
		}else if (errn == DUPLICATE_LISTEN){
			os << "webserv: deuplicated listen '" << arg << "'" << std::endl;
			throw std::invalid_argument(buffer.str());
		}else if (errn == FILE_DOES_NOT_EXIST){
			os << "webserv: file does not exist '" << arg << "'" << std::endl;
			throw std::invalid_argument(buffer.str());
		}else if (errn == PARSING_ERROR){
			os << "webserv: Parsing error '" << arg << "'" << std::endl;
			throw std::invalid_argument(buffer.str());
		}
	}
}


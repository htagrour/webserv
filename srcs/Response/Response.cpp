#include "Response.hpp"

void findAndReplace(std::string &s, char f, const char *r)
{
    size_t p;
   while ( (p = s.find(f)) != s.npos )
   {
      s.replace(p, 1, r);
   }
}

std::string toRawString(std::string const& in)
{
   std::string ret = in;

   findAndReplace(ret, '\t', "\\t");
   findAndReplace(ret, '\n', "\\n");
   findAndReplace(ret, '\r', "\\r");

   return ret;
}

const char *	ft::Response::FileNotFound::what() const throw()
{
	return ("file body not found");	
}

const char *	ft::Response::EndOfBuffer::what() const throw()
{
	return ("END OF BUFFER REACHED");
}

ft::Response::Response()
{
	_methods.insert(std::make_pair(GET, &Response::getMethod));
	_methods.insert(std::make_pair(POST, &Response::postMethod));
	_methods.insert(std::make_pair(DELETE, &Response::deleteMethod));
	_methods.insert(std::make_pair(UNKNOWN, &Response::unknownMethod));
	_excuted = false;
	_complted = false;
	_HeaderSend = false;
	_isCgi = false;
	sended = 0;
	_ContentLength = 0;
	_bodyRead = 0;
}

void fun(const ft::Request& req)
{
    std::cout << "==================================\n";
    std::cout << "REQUEST: " << std::endl;
    std::cout << "URL: " << req.getUrl() << "\n";
    std::cout <<  "METHOD: " << req.getMethod() << "\n";
    std::cout << "HTTPVERSION: " << req.getHttpVersion() << "\n";
    std::cout << "ATTRIBUTES: \n";
    for (std::map<std::string, std::string>::const_iterator iter = req.getAttributes().begin(); iter != req.getAttributes().end(); iter++)
    {
        std::cout << iter->first << " " << iter->second << std::endl;
    }
   try
    {
        std::cout << "fileName: " << req.getFileName() << "\n";
    }
    catch(const std::exception & e)
    {
        std::cerr << e.what() << '\n';
    }
    std::cout << "=========================================\n";
}

void			ft::Response::executeRequest(ft::Request& req)
{
	if(_excuted)
		return;
	Checker checker(req);
	_status_code = checker.getStatusCode();
	if (_status_code == 200 || checker.isRedirect())
	{
		if(checker.isCgi())
		{
			Cgi cgi(checker,req);

			if(cgi.executeCgi(checker, req))
				cgi.parseCgiFile(*this);
			else
				sendError(checker, 500);
		}
		else
			(this->*_methods[req.getMethod()])(req,checker);
	}
	else
		sendError(checker,_status_code);
	_http_version = HTTP_VERSION;
	_reason_phrase = HttpStatus::reasonPhrase(_status_code);
	AddHeaders("Connection", "close");
	AddHeaders("Date", getCurrentTime());
	generateResponseBuffer();
	_excuted = true;
}

ft::Response::~Response()
{
	if (_isCgi)
		std::remove(_cgiPath.c_str());
}

void		ft::Response::setCgi()
{
	_isCgi = true;
}

bool		ft::Response::getLineFromBody(std::string & str)
{

	std::getline(_body, str);
	return (_body.good());
}

void		ft::Response::sendSuccessed(size_t length)
{
	sended += length;
	if (!_HeaderSend)
	{
		_HeaderSend = sended >= _bufferToSend.length();
		_complted = _HeaderSend && !_ContentLength;
	}
	else
	{
		_complted = (sended >= _bodyRead) && _body.eof();
		if (_complted)
			_body.close();
	}
}

void		ft::Response::generateResponseBuffer(void)
{
	_buffer << _http_version << " " << _status_code << " " << _reason_phrase << "\r\n";
	if (!_HeaderSend)
	{
		for (std::map<std::string, std::string>::iterator headerIterator = _headers.begin(); headerIterator != _headers.end(); headerIterator++)
		{
			_buffer << headerIterator->first << ": " << headerIterator->second << "\r\n";
		}	
		_buffer << "\r\n";
		_bufferToSend = _buffer.str();
		_bodyRead = _bufferToSend.length();
	}
}

void					ft::Response::AddHeaders(const std::string & key, const std::string & value)
{
	_headers.insert(std::make_pair(key, value));	
}

void					ft::Response::AddResponseBody(const char *path) throw(FileNotFound)
{
	
	std::stringstream	converter;

	_ContentLength = getFileSize(path);
	if (_ContentLength == -1)
		throw FileNotFound();
	_body.open(path,  std::ifstream::binary);
	converter << _ContentLength;
	AddHeaders("Content-Length", converter.str());
	// AddHeaders("Transfer-Encoding", "chunked");
	if (!_isCgi)
	{
		AddHeaders("Content-Type", getContentType(path));
		AddHeaders("Last-Modified", getFileModifiedDate(path));
	}
	else
		_cgiPath = path;
}
 

bool				ft::Response::isCompleted(void) const
{
	return (_complted);
}

std::string			ft::Response::readFromBodyFile(size_t Size)
{
	char			*buffer;
	std::stringstream	read;
	if (!_body.eof())
	{
		buffer = new char[Size + 1];
		_body.read(buffer, Size);
		read.write(buffer, _body.gcount());
		_bodyRead = _body.gcount();
		delete[] buffer;
	}
	return (read.str());
}

std::string	ft::Response::getBuffer(size_t Size)
{
	std::string		toSend;

	toSend = "";
	if (!_HeaderSend)
		toSend = _bufferToSend.substr(sended, std::min(Size, _bufferToSend.length()));
	else
	{
		if (sended >= _bufferToSend.length())
		{
			sended = 0;
			_bufferToSend = readFromBodyFile(Size);
		}
		toSend = _bufferToSend.substr(sended, std::min(Size, _bodyRead));
	}
	return (toSend);
}

void	ft::Response::sendError(Checker &checker,int statusCode)
{
	std::string path;

	path = checker.getLocation().getErrorPage()[statusCode];
	try
	{
		AddResponseBody(path.c_str());
	}
	catch(const std::exception& e)
	{
		generateErrorPage(statusCode);
	}
	_status_code = statusCode;
}

void				ft::Response::setStatusCode(int statusCode)
{
	_status_code = statusCode;
}

// Methods

void	ft::Response::getMethod(const Request& req, Checker & checker)
{
	std::cout << "Get Method Triggered" << std::endl;
	char *pathStr = getcwd(NULL, 1024);
	std::string path(pathStr);
	PathStatus pathStatus;

	delete[] pathStr;
	if (checker.isRedirect())
	{
		_status_code = checker.getLocation().getRedirect().first;
		AddHeaders("Location", checker.getLocation().getRedirect().second);
		return;
	}

	path = checker.getRoot();
	pathStatus = isFile(path);
	if(pathStatus == FIL)
	{
		_status_code = 200;
		AddResponseBody(path.c_str());
		return;
	}
	
	if (pathStatus == DIRC)
	{
		if (checker.isAutoIndex())
		{
			_status_code = 200;
			genarateFolderList(path.c_str(), req);
			return;
		}
		_status_code = 403;
	}
	sendError(checker,_status_code);
}

void	ft::Response::postMethod(const Request& req, Checker & checker)
{
	std::cout << "Post Method Triggered" << std::endl;
	std::string type;
	std::string boundry;
	std::map<std::string, std::string>::const_iterator it;
	int code = 403;

	if(checker.getLocation().getUpload())
	{
		it = req.getAttributes().find("Content-Type");
		if(it != req.getAttributes().end())
		{
			std::istringstream is(it->second);
			std::getline(is, type, ';');
			type = type.substr(1);
			if (type == "multipart/form-data")
			{
				std::getline(is, boundry);
				boundry = boundry.substr(10);
				boundry = "--" + boundry;
				uploadFile(req, checker, boundry);
				return;
			}
		}
		code = 400;
	}
	sendError(checker,code);
}

std::string getFileName(std::string line)
{
	size_t pos;
	pos = line.find("filename");
	if (pos != std::string::npos)
	{
		return line.substr(pos + 10);
	}
	return "";
}

void createFile(std::string path, std::ifstream& fileBody, std::string boundry)
{
	std::ofstream file(path.c_str());
	std::string line;

	while(std::getline(fileBody, line))
	{
		if (line == boundry + "\r" || line == boundry + "--\r")
			break;
		file  << line << std::endl;
	}
	file.close();
}

void ft::Response::uploadFile(const Request& req, Checker &checker,const std::string &boundry)
{
	std::ifstream fileBody(req.getFileName().c_str());
	std::string line;
	std::string fileName;
	bool ikhan = false;

	while(std::getline(fileBody, line))
	{
		if (line == boundry + "\r")
			continue;
		if (!ikhan)
		{
			if(line.find("Content-Disposition") != std::string::npos)
			{
				fileName = getFileName(line);
				fileName = fileName.substr(0, fileName.length() - 2);
				if (!(fileName).length())
				{
					_status_code = 400;
					break;
				}
			}
			if (line.find("Content-Type") != std::string::npos)
				ikhan = true;
		}
		else
		{
			ikhan = false;
			createFile(checker.getLocation().getRoot() + "/" + fileName, fileBody, boundry);
			if (checker.isRedirect())
			{
				_status_code = checker.getLocation().getRedirect().first;
				AddHeaders("Location", checker.getLocation().getRedirect().second);
				return;
			}
			else
			{
				_status_code = 201;
				AddHeaders("Content-Length","0");
			}

		}
	}
	fileBody.close();
}

void	ft::Response::deleteMethod(const Request& req, Checker & checker)
{
	(void)req;
	std::cout << "Delete Method Triggered" << std::endl;

	std::string path = checker.getRoot();

	if(isFile(path) == FIL)
	{
		std::remove(path.c_str());
	}
	if (checker.isRedirect())
	{
		_status_code = checker.getLocation().getRedirect().first;
		AddHeaders("Location", checker.getLocation().getRedirect().second);
		return;
	}
	else
		_status_code = 204;
}

void	ft::Response::unknownMethod(const Request& req,Checker &checker)
{
	(void)req;
	(void)checker;
	std::cout << "Unkonw Method!!!" << std::endl;
}


PathStatus	ft::Response::isFile(const std::string &path)
{
	struct stat s;

	if (stat(path.c_str(),&s))
		return NOTEX;
	if (!S_ISDIR(s.st_mode))
		return FIL;
	return DIRC;
}

void ft::Response::genarateFolderList(const char *str, const Request& req)
{
	struct dirent *d;
    DIR *dr;
    dr = opendir(str);
	int i = 0;
	const char * listPath = "/tmp/list.html";
	std::ofstream html(listPath);
    if(dr!=NULL)
    {
        for(d=readdir(dr); d!=NULL; d=readdir(dr), i++)
        {
			if (i > 1)
			{
				html << "<a "
						<<"href=" << "\"http://" << req.getAttributes().find("Host")->second.substr(1);
				if (req.getUrl() != "/" )
					html << req.getUrl();
				html << "/" << d->d_name << "\"" <<  ">"
					<<"<h3>" << d->d_name
					<< "</h3></a>";
			}
        }
        closedir(dr);
    }
	html.close();
	AddResponseBody(listPath);
	std::remove(listPath);
}

void  ft::Response::generateErrorPage(int statusCode)
{
	std::ofstream os("/tmp/ikhan.html");
	os << "<!DOCTYPE html>" << std::endl;
	os << "<html lang=\"en\">" << std::endl;
	os << "<head>" << std::endl;
	os << "<title>" << std::endl;
	os << HttpStatus::reasonPhrase(statusCode) << std::endl;
	os << "</title>" << std::endl;
	os << "</head>" << std::endl;
	os << "<body>" << std::endl;
	os << HttpStatus::reasonPhrase(statusCode) << std::endl;
	os << "</body>" << std::endl;
	os << "</html>" << std::endl;

	AddResponseBody("/tmp/ikhan.html");
	std::remove("/tmp/ikhan.html");
}
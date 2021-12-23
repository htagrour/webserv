

#include "checker.hpp"

const char  * ft::Checker::UrlNotFound::what() const throw()
{
	return ("THE GIVEN URL NOT EXIST");
}

ft::Checker::Checker(Request & req)
{
	_status_code = 200;
	_isCgi = false;
	_is_redirect = false;
	// looking for server based on host
	_server = req.getServer();
	if (req.CheckBodyLimit())
	{
		_status_code = 413;
		_root = _location.getErrorPage()[_status_code];
		return ;
	}
	// looking for Location
	try
	{
		findLocation(_server.getLocation(), req.getUrl());
		if (!CheckIndexExistance(req.getIndex()))
		{
			if (!_location.getAutoIndex())
			{
				if (_location.getIndex().size())
				{
					_root = removeSlash(_root) + "/" + getValidIndex();
					_isCgi = _location.getCgi().length();
				}
				else if (!(req.getMethod() == POST && _location.getUpload()))
				{
					_status_code = 403;
					_root = _location.getErrorPage()[_status_code];
				}
			}
			else if (_location.getAutoIndex())
				_is_auto_index = true;
		}
		else
		{
			_root = removeSlash(_root);
			_isCgi = _location.getCgi().length();
		}
		if (_location.getRedirect().first != HttpStatus::None)
		{
			_is_redirect = true;
			_status_code = _location.getRedirect().first;
		}
		else if (!isCgi() && !checkMethod(req.getMethod(), _location.getMethods()))
		{
			_status_code = 405;
			_root = _location.getErrorPage()[_status_code];
		};
	}
	catch(const std::exception& e)
	{
		_status_code = 404;
		_root = _location.getErrorPage()[_status_code];
	}
	if (_status_code == 200 && !checkRootExist())
	{
		_status_code = 404;
		_root = _location.getErrorPage()[_status_code];
	}
	_is_auto_index = _location.getAutoIndex();
}



void				ft::Checker::findLocation(std::map<std::string, Server> & locations, const std::string & requestLocation) throw(UrlNotFound)
{
	std::string     mostAccurateLocationStr= "";
	Server          mostAccurateLocationServer;
	std::string		cleanedLocationStr;
	int				beginEnd;

	cleanedLocationStr = addSlash(requestLocation);
	for (std::map<std::string, Server>::iterator iter = locations.begin(); iter != locations.end(); iter++)
	{
		beginEnd = cleanedLocationStr.length() - iter->first.length() - 1;
		beginEnd = (beginEnd < 0) ? 0 : beginEnd;
		if (iter->second.getCgi().length() && iter->first.length() <= cleanedLocationStr.length() && cleanedLocationStr.substr(beginEnd).find(iter->first) != std::string::npos)
		{
			mostAccurateLocationServer = iter->second;
			mostAccurateLocationStr = iter->first;
		}
		if (cleanedLocationStr.substr(0, iter->first.length()).find(iter->first) == std::string::npos)
			continue ;
		if (mostAccurateLocationStr.length() < iter->first.length())
		{
			mostAccurateLocationStr = iter->first;
			mostAccurateLocationServer = iter->second;
		}
	}
	if (!mostAccurateLocationStr.length())
		throw UrlNotFound();
	_location = mostAccurateLocationServer;
	if (mostAccurateLocationServer.getCgi().length())
	{
		_root =  removeSlash(mostAccurateLocationServer.getRoot()) +  SlashBeforeConcat(requestLocation) +  requestLocation;
	}
	else
	{
		cleanedLocationStr = cleanedLocationStr.substr(mostAccurateLocationStr.length());
		_root = removeSlash(mostAccurateLocationServer.getRoot()) + SlashBeforeConcat(cleanedLocationStr) + cleanedLocationStr;
	}
}


bool            ft::Checker::checkMethod(Method method, std::set<Method> & methods)
{
	return (methods.find(method) != methods.end());
}

// GETTERS

bool            ft::Checker::isCgi() const
{
	return (_isCgi);
}

int             ft::Checker::getStatusCode() const
{
	return (_status_code);
}

const std::string & ft::Checker::getRoot() const
{
	return (_root);
}

bool			ft::Checker::isAutoIndex() const
{
	return (_is_auto_index);
}

bool			ft::Checker::checkRootExist(void) const
{
	struct stat attrs;
	return (stat(_root.c_str(), & attrs) == 0);
}

bool			ft::Checker::CheckIndexExistance(const std::string & index)
{
	return (index.length() > 0 && _location.getUri().find(index) == std::string::npos);
}

std::string		ft::Checker::getValidIndex()
{
	std::string		temp_root;

	temp_root = _root;
	for (std::vector<std::string>::iterator indexIter = _location.getIndex().begin(); indexIter != _location.getIndex().end(); indexIter++)
	{
		_root = temp_root + "/" + *indexIter;
		if (checkRootExist())
		{
			_root = temp_root;
			return (*indexIter);
		}
	}
	_root = temp_root;
	return (_location.getIndex()[0]);
}

ft::Server		ft::Checker::getLocation()
{
	return (_location);
}

bool			ft::Checker::isRedirect() const
{
	return (_is_redirect);
}
std::ostream 	& ft::operator<<(std::ostream & os, const Checker & check)
{
	os << "ROOT: " << check.getRoot() << std::endl;
	os << "STATUS CODE: " << check.getStatusCode() << std::endl;
	os << "AutoIndex: " << check.isAutoIndex() << std::endl;
	os << "ISCGI: " << check.isCgi() << std::endl;
	return (os);
}
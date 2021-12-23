#include "Request.hpp"

namespace ft
{

    // REQUEST EXCEPTION
    const char * Request::FileNotExist::what() const throw() 
    {
        return ("FILE NOT EXIST FOR THIS REQUEST");
    }


    // REQUEST CONSTRUCTOR
    Request::Request(void)
    {
        line = 0;
        currentBodySize = 0;
        isCgi = true;
        currentChunkedLength = String::pos;
        reachedLength = 0;
        http_version = "";
        contentLength = String::pos;
        isFileCreated = false;
        empty_line = false;
        _body_reach_limit = false;
        _body_size_limit = std::string::npos;
        file = "/tmp/" + randomString(16);
    }

    //DESTRACTOR NO NEEDED FOR NOW!
    Request::~Request()
    {
        if (contentLength != String::pos)
        {
            //IN CASE OF TEST JUST COMMENT THE LINE BELOW SO THE FILE NOT DELETED ON A tmp DIRECTORY
            std::remove(file.c_str());
        }
    }


    // FUNCTION PARSE GAVEN BUFFER RETURN WETHER REACH END OF FILE OR NOT
    bool         Request::parseBuffer(const char * buffer, size_t bufferSize)
    {
        int                                                 notEndOfLine;
        std::map<std::string, std::string>::const_iterator  iter;
        notEndOfLine = false;
        String currentBuffer(buffer, bufferSize);
        while (!notEndOfLine)
        {
            if (line == 0)
            notEndOfLine = ParseFirstLineHeader(currentBuffer);
            else if (!empty_line)
                notEndOfLine = ParseAttribute(currentBuffer);
            else
            {
                setCorrespondingServer();
                iter = attributes.find("Transfer-Encoding");
                if (iter != attributes.end() && iter->second == " chunked")
                {
                    if (_server.getMaxBodySize() !=  std::string::npos && currentBodySize > _server.getMaxBodySize())
                    {
                        _body_reach_limit = true;
                        return (true);
                    }
                    return ParseChunkedBody(currentBuffer);
                }
                iter = attributes.find("Content-Length");
                if (iter != attributes.end())
                {
                    contentLength = std::atoll(iter->second.c_str());
                    if (_server.getMaxBodySize() !=  std::string::npos && contentLength > _server.getMaxBodySize())
                    {
                        _body_reach_limit = true;
                        return (true);
                    }
                    return (parseBody(currentBuffer));
                }
                return (true);
            }
        }
        return (false);
    }

    // PARSER OF THE FIRST LINE
    int         Request::ParseFirstLineHeader(String & buffer)
    {
        std::string  method;
        size_t      endofline;
        
        endofline = buffer.findPosition("\r\n");
        if (endofline == String::pos)
        {
            StoredBuffer += buffer;
            return (1);
        }
        StoredBuffer += buffer.substr(0, endofline);
        std::stringstream s(StoredBuffer);
        s >> method >> url >> http_version;
        setMethod(method);
        StoredBuffer = "";
        buffer += endofline + 2;
        line++;
        parseUrl();
        return (!buffer.length());
    }
    // PARSING ATTRIBUTES

    int       Request::ParseAttribute(String & buffer)
    {
        std::string sline;
        std::string key, value;
        String  sub;
        size_t     endofline;
        
        endofline = buffer.findPosition("\r\n");
        if (endofline == String::pos)
        {
            StoredBuffer += buffer;
            return (1);
        }
        sub = buffer.substr(0, endofline);
        if (!sub.length())
        {
            empty_line = true;
            buffer += endofline  + 2;
            return (0);
        }
        StoredBuffer += sub;
        std::stringstream s(StoredBuffer);
        std::getline(s, sline);
        key = sline.substr(0, sline.find(':'));
        value = sline.substr(sline.find(':') + 1);
        attributes.insert(std::make_pair(key, value));
        buffer += endofline + 2;
        line++;
        StoredBuffer = "";
        return (0);
    }

    // PARSING BODY
    bool        Request::parseBody(String & buffer)
    {
        if (!BodyFile.is_open())
            BodyFile.open(file, std::ios::out | std::ios::binary);
        BodyFile.write(buffer.base(), buffer.length());
        currentBodySize += buffer.length();
        if (currentBodySize >= contentLength)
            BodyFile.close();
        return (currentBodySize >= contentLength);
    }

    //PARSING BODY ENCODING CHUNKED
    bool    Request::ParseChunkedBody(String & buffer)
    {
	    std::stringstream	hex;
	    size_t				newLinePosition;
        std::string         toWrite;
        size_t              currentPosition;
        StoredChunkedBuffer.write(buffer, buffer.length());
        if (!BodyFile.is_open())
            BodyFile.open(file, std::ios::out | std::ios::binary);
	    while (true)
	    {
		    if (currentChunkedLength == std::string::npos|| reachedLength >= currentChunkedLength)
		    {
			    currentChunkedLength = 0;
			    reachedLength = 0;
			    hex.clear();
			    newLinePosition = StoredChunkedBuffer.str().find("\r\n");
			    if (newLinePosition == std::string::npos)
                {
				    break;
                }
			    hex << std::hex << StoredChunkedBuffer.str().substr(0, newLinePosition);
			    hex >> currentChunkedLength;
			    if (!currentChunkedLength)
                {
                    BodyFile.close();
				    return (true);
                }
                currentBodySize += currentChunkedLength;
                StoredChunkedBuffer.str(StoredChunkedBuffer.str().substr(newLinePosition + 2));
			    if (!StoredChunkedBuffer.str().length())
				    break;
		    }
            else
            {
                toWrite = StoredChunkedBuffer.str().substr(0, std::min(currentChunkedLength - reachedLength, StoredChunkedBuffer.str().length()));
                BodyFile.write(toWrite.c_str(), toWrite.length());
                StoredChunkedBuffer.str(StoredChunkedBuffer.str().substr(toWrite.length()));
                reachedLength += toWrite.length();
                currentPosition = StoredChunkedBuffer.tellg();
                if (StoredChunkedBuffer.str().find("\r\n") == 0) // CHECK WHETHER THE StoredBuffer START WITH CRLF
                    StoredChunkedBuffer.str(StoredChunkedBuffer.str().substr(2));
                if (!StoredChunkedBuffer.str().length())
                    break;
            }
        }
        return (false);
}
    // getters
    Method      Request::getMethod() const
    {
        return (method);
    }

    const std::map<std::string, std::string> & Request::getAttributes() const
    {
        return (attributes);
    }

    const std::string       & Request::getHttpVersion() const
    {
        return (http_version);
    }

    const std::string       & Request::getUrl() const
    {
        return (url);
    }

    bool                        Request::CheckBodyLimit() const
    {
        return (_body_reach_limit);
    }
    bool                      Request::getIsCgi() const
    {
        return isCgi;
    }
    Server                      Request::getServer()
    {
        return _server;
    }
    // FUNCTION RETURN [fileName] in case  there are body otherways FileNotExist Exception are thrown;
    const std::string       & Request::getFileName() const throw(FileNotExist)
    {
        if (attributes.find("Content-Length") == attributes.end() && attributes.find("Transfer-Encoding") ==  attributes.end())
            throw(FileNotExist());
        return (file);
    }

    size_t                  Request::getContentLength() const
    {
        return (contentLength);
    }

    // method Setters

    void        Request::setMethod(const std::string & str)
    {
        if (toUpperCase(str) == "GET")
            method = GET;
        else if (toUpperCase(str) == "POST")
            method = POST;
        else if (toUpperCase(str) == "DELETE")
            method = DELETE;
        else
            method = UNKNOWN;
    }

    void  Request::setBodyLimit(size_t limit)
    {
        _body_size_limit = limit;
    }

    void       Request::setServers(std::vector<Server> servers)
    {
        _all_servers = servers;
    }
    void        Request::findServerBasedOnHost(std::vector<Server> & servers, const std::string & host)
    {
	    std::string     cleanedHost;

	    cleanedHost = host.substr(1); // REMOVING SPACE IN THE START
	    for (std::vector<Server>::iterator iter = servers.begin(); iter != servers.end(); iter++)
	    {
		    if (iter->getServerName().length())
		    {
			    if (iter->getServerName() == cleanedHost)
			    {
				    _server = *iter;
				    return ;
			    }
		    }
	    }
	    _server = servers[0];
    }

    void Request::setCorrespondingServer()
    {
	    if (getAttributes().find("Host") != getAttributes().end())
		        findServerBasedOnHost(_all_servers, getAttributes().find("Host")->second);
	    else
		    _server = *_all_servers.begin();
    }

    void    Request::parseUrl()
    {
        size_t      queryStartPosition;
        size_t      indexStartPosition;

        url = handleSpace(url);
        indexStartPosition = url.find_last_of('/');
        queryStartPosition = url.find('?');
        if (queryStartPosition != std::string::npos)
        {
            query = url.substr(queryStartPosition + 1);
            url = url.substr(0, queryStartPosition);
            index = url.substr(indexStartPosition + 1, queryStartPosition - indexStartPosition - 1);
        }
        else
            index = url.substr(indexStartPosition + 1);
        root  = url.substr(0, queryStartPosition);
    }

    const std::string   & Request::getPath() const
    {
        return (root);
    }

    const std::string   & Request::getIndex() const
    {
        return (index);
    }

    const std::string   & Request::getQuery() const
    {
        return (query);
    }
    int                 Request::getListenSocket() const
    {
        return (listenSocket);
    }
    void    Request::setCgi()
    {
        isCgi = true;
    }
}

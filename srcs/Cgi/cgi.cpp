#include "cgi.hpp"

extern char** environ;
ft::Cgi::Cgi(Checker &checker, const Request& req)
{

    initCgi(checker, req);
}

void ft::Cgi::initCgi(Checker &checker, const Request& req)
{
    std::stringstream os;

    os << checker.getLocation().getPort();
    setenv("SERVER_PORT", os.str().c_str() , 1);
    setenv("SCRIPT_NAME",  checker.getLocation().getCgi().c_str(), 1);
    setenv("SERVER_PROTOCOL",  "HTTP/1.1", 1);
    os.clear();
    os << req.getContentLength();
    if (req.getContentLength() != std::string::npos)
        setenv("CONTENT_LENGTH",os.str().c_str(),1);
    else
        setenv("CONTENT_LENGTH", "",1);
    setenv("QUERY_STRING",  req.getQuery().c_str(), 1);
    setenv("PATH_INFO", req.getPath().c_str(), 1);
    setenv("GATEWAY_INTERFACE",  "CGI/1.1", 1);
    setenv("SERVER_NAME",  checker.getLocation().getServerName().c_str(), 1);
    setenv("SERVER_SOFTWARE",  "IKHAN/1.0", 1);
    setenv("REQUEST_METHOD",  getMethodName(req.getMethod()).c_str() , 1);
    setenv("SCRIPT_FILENAME", checker.getRoot().c_str(), 1);
    setenv("REDIRECT_STATUS", "200", 1);
    std::map<std::string, std::string>::const_iterator cookie = req.getAttributes().find("Cookie");
    if(cookie != req.getAttributes().end())
        setenv("HTTP_COOKIE", cookie->second.substr(1).c_str(), 1);
}

bool ft::Cgi::executeCgi(Checker &checker, const Request& req)
{
    int ret;
    int pid;

    outFile = open(CGIOUTPUT, O_CREAT | O_RDWR, 0644);
    inputFile = 0;
    argv = (char**)malloc(sizeof(char*) * 3);
    if (req.getContentLength() != std::string::npos)
        inputFile = open(req.getFileName().c_str(), O_RDONLY);
    if(outFile == -1 || inputFile == -1 || !argv)
        return false;
    argv[0] = strdup(checker.getLocation().getCgi().c_str());
    argv[1] = strdup(checker.getRoot().c_str());
    argv[2] = NULL;

    if (!argv[0] || !argv[1])
        return false;
    
    if ((pid = fork()) == -1)
        return false;
    if (pid == 0)
    {
        dup2(outFile, 1);
        dup2(inputFile, 0);
        execve(*argv,argv,environ);
        std::cout << IKHANERR << std::endl;
        exit(1);
    }
    else
    {
        waitpid(pid, &ret,0);
        close(outFile);
        ret = WEXITSTATUS(ret);
    }
    return (!ret);
}

void ft::Cgi::parseCgiFile(Response& res)
{
    std::ifstream cgiOutPut(CGIOUTPUT);
    std::string line;
    std::string key;
    std::string value;
    std::string fileOutput;
    std::ofstream output;
    size_t pos;


    fileOutput = "/tmp/cgiBody-" + randomString(15);
    output.open(fileOutput);
    res.setCgi();
    while(std::getline(cgiOutPut,line) && line != "\r")
    {
        pos = line.find(':');
        key = line.substr(0, pos);
        value = line.substr(pos+2, line.length());
        res.AddHeaders(key.c_str(), value.c_str());
    }
    while(std::getline(cgiOutPut, line))
    {
        output << line;
    }
    output.clear();
    output.seekp(0);
    output.close();
    res.AddResponseBody(fileOutput.c_str());
    cgiOutPut.close();
}

ft::Cgi::~Cgi()
{
    std::remove(CGIOUTPUT);
    if (outFile)
        close(outFile);
    if (inputFile && inputFile != -1)
        close(inputFile);
    delete[] argv;
}
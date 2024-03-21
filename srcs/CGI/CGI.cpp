#include "CGI.hpp"

void    CGI::getPathInfo()
{
    std::string rel_path(_request->getURN().substr(_route->getRoute().length()));
    _env["PATH_INFO"] = "";
    _env["PATH_INFO"] += _route->getRoot();
    _env["PATH_INFO"] += rel_path;
}

void    CGI::getQueryString()
{
    _env["QUERY_STRING"] = "";
    for (std::map<std::string, std::string>::const_iterator ite = _request->getURLParams().begin();
        ite != _request->getURLParams().end();
        ite++)
    {
        _env["QUERY_STRING"] += ite->first;
        _env["QUERY_STRING"] += '=';
        _env["QUERY_STRING"] += ite->second;
        if (++ite != _request->getURLParams().end())
            _env["QUERY_STRING"] += '&';
        ite--;
    }
}

void    CGI::getContentLength()
{
    std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().find("Content-length");

    _env["CONTENT_LENGTH"] = "";
    if (ite == _request->getHeaders().end())
    {
        std::stringstream ss;

        ss << _request->getBody().length();
        _env["CONTENT_LENGTH"]  = ss.str();
        return ;
    }
    _env["CONTENT_LENGTH"] = ite->second;
}

void    CGI::getRequestMethod()
{
    _env["REQUEST_METHOD"]  = Config::perm2str(_request->getMethod());
}

void    CGI::getContentType()
{
    _env["CONTENT_TYPE"] = _request->findHeader("Content-type");
}

void    CGI::getServerName(const ServerConf &server)
{
    std::map<std::string, std::string>::const_iterator  header_ite  = _request->getHeaders().find("Host");
    std::vector<std::string>::const_iterator            sname_ite   = std::find(server.getNames().begin(), server.getNames().end(),  header_ite->second);

    _env["SERVER_NAME"] = "";
    if (header_ite == _request->getHeaders().end()
        || sname_ite == server.getNames().end())
        return ;
    _env["SERVER_NAME"] = header_ite->second;
}

static std::string  addPrefixCapitalize(std::string key)
{
    std::string str("HTTP_");

    for (size_t i = 0; i < key.size(); i++)
    {
        if (std::isalpha(key[i]))
            str += std::toupper(key[i]);
        else
            str += "_";
    }
    return str;
}

void    CGI::getHeaders()
{
    for (std::map<std::string, std::string>::const_iterator ite = _request->getHeaders().begin();
        ite != _request->getHeaders().end();
        ite++)
        _env[addPrefixCapitalize(ite->first)] = ite->second;
}


void    CGI::setCGI(std::string cgiPath)
{
    _cgi_path = cgiPath;
}


CGI::CGI() : _env_execve(NULL)
{
}

CGI::~CGI()
{
    if (_env_execve != NULL)
    {
        for (size_t i = 0; i < _env.size(); i++)
            delete _env_execve[i];
        delete[] _env_execve;
    }
}

void    CGI::prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr)
{
    std::string query_string;
    std::string path_info;
    std::stringstream ss;

    _request    = &req;
    _route      = &route;
    _status     = 200;
    _env.clear();
    getPathInfo();
    getQueryString();
    getContentLength();
    getRequestMethod();
    getContentType();
    getServerName(server);
    getHeaders();
    _env["REDIRECT_STATUS"] = "true";
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["SCRIPT_FILENAME"] = _env["PATH_INFO"];
    _env["REMOTE_ADDR"] = remoteaddr;
    _env["REMOTE_HOST"] = "";
    _env["REMOTE_IDENT"] = "";
    _env["REMOTE_USER"] = "";
    ss << server.getPort();
    _env["SERVER_PORT"] = ss.str();
    ss.clear();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "webserv/1.0";
}

static char *concatcpy(std::string key, std::string value)
{
    char    *ret;
    size_t  i = 0;
    size_t  j = 0;

    ret = new char[key.length() + value.length() + 2];
    for (j = 0; j < key.length(); j++)
    {
        ret[i] = key[j];
        i++;
    }
    ret[i] = '=';
    i++;
    for (j = 0; j < value.length(); j++, i++)
        ret[i] = value[j];
    ret[i] = 0;
    return ret;
}

char    **CGI::buildEnvFromAttr()
{
    char    **ret;
    size_t  i = 0;

    ret = new char*[_env.size()+1];
    for (std::map<std::string, std::string>::const_iterator ite = _env.begin();
        ite != _env.end();
        ite++)
    {
        ret[i] = concatcpy(ite->first, ite->second);
        i++;
    }
    ret[i] = NULL;
    return ret;
}

void    CGI::childProc(int fds[2])
{
    char        *av[] = {NULL, NULL};

    dup2(fds[1], STDOUT_FILENO);
    dup2(fds[0], STDIN_FILENO);
    close(fds[0]);
    close(fds[1]);
    _env_execve = buildEnvFromAttr();
    execve(_cgi_path.c_str(), av, _env_execve);
    exit(127);
}

static void waitTimeout(pid_t pid)
{
    int     s;
    int     ret;
    clock_t start;

    start = clock();
    s = 0;
    while ((ret = waitpid(pid, &s, WNOHANG)) == 0)
    {
        if (((clock() - start) / CLOCKS_PER_SEC) >= GATEWAY_TIMEOUT )
        {
            kill(pid, SIGSTOP);
            throw std::runtime_error("504");
        }
    }
    if (ret < 0)
        throw std::runtime_error("500");
    if (WIFEXITED(s) && WEXITSTATUS(s) != 0)
        throw std::runtime_error("503");
}

void    CGI::parentProc(int fds[2], pid_t pid)
{
    char    c;
    ssize_t rd;

    _raw_response.clear();
    if (_env["REQUEST_METHOD"] == "POST")
        write(fds[1], _request->getBody().c_str(), _request->getBody().length());
    close(fds[1]);
    waitTimeout(pid);
    do {
        rd = read(fds[0], &c, 1);
        if (rd < 0)
            throw std::runtime_error("500");
        _raw_response += c;
    } while (rd > 0);
    close(fds[0]);
    parseRaw();
}

void    CGI::forwardReq()
{
    int         fds[2];
    pid_t       pid;

    _headers.clear();
    if (_env.size() == 0)
        throw std::runtime_error("500");
    if (pipe(fds) < 0)
        throw std::runtime_error("500");
    pid = fork();
    if (pid == -1)
        throw std::runtime_error("500");
    if (pid == 0)
        childProc(fds);
    else
        parentProc(fds, pid);
    _env.clear();
}

void    CGI::parseRaw()
{
    std::vector<std::string> out;
    std::stringstream   ss(_raw_response);
    std::string         line;

    _body.clear();
    _headers.clear();
    while (getlineCRLF(ss, line))
    {
        if (line.length() == 0)
            break;
        split(line, out, ':');
        if (out.size() != 2)
            continue;
        trimstr(out[0]); trimstr(out[1]);
        _headers[out[0]] = out[1];
        out.clear();
    }
    std::map<std::string, std::string>::const_iterator ite = _headers.find("Status");
    if (ite != _headers.end())
    {
        _status = std::strtol(ite->second.c_str(), NULL, 10);
        _headers.erase(ite->first);
    }
    if (ss.tellg() != -1)
        _body = ss.str().substr(ss.tellg());
    
}

int CGI::getStatus() const
{
    return _status;
}

const std::string   &CGI::getRawResp() const
{
    return _raw_response;
}

const std::string   &CGI::getBody() const
{
    return _body;
}

std::string         CGI::buildRawHeader() const
{
    std::map<std::string, std::string>::const_iterator ite = _headers.begin();
    std::string ret;

    for (;ite != _headers.end(); ite++)
    {
        ret += ite->first;
        ret += ": ";
        ret += ite->second;
        ret += "\r\n";
    }
    return ret;
}


const std::map<std::string, std::string> &CGI::getHeaders() const
{
    return _headers;
}

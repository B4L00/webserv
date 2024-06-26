#ifndef _CGI_
#define _CGI_

#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <sys/wait.h>
#include <cstring>
#include <fcntl.h>
#include "Request.hpp"
#include "Route.hpp"

#define GATEWAY_TIMEOUT 5.0

class CGI
{
    private :
        const Request       *_request;
        const Route         *_route;
        std::map<std::string, std::string>  _env;
        std::map<std::string, std::string>  _headers;
        int                                 _status;
        std::string         _body;
        std::string         _cgi_path;
        std::string         _raw_response;
        char                **_env_execve;

        pid_t               _pid;
        int                 _fds_in[2];
        int                 _fds_out[2];
        bool                _is_started;
        ssize_t              _bdc;

        CGI(CGI const &a);
        CGI  &operator=(CGI const &a);

        void    getQueryString();
        void    getContentLength();
        void    getContentType();
        void    getRequestMethod();
        void    getServerName();
        void    getHeaders();
        void    parentProc();
        void    childProc();

        void    parseHeader(std::stringstream &ss);
        void    parseRaw();
        char    **buildEnvFromAttr();

        void    readAllWait();
        void    freeExecEnv();
    public :
        ~CGI();
        CGI();

        void        prepare(Request const &req,
                        Route const &route,
                        ServerConf const &server,
                        std::string remoteaddr,
                        std::string path_info);
        void        start();
        bool        receive(const char *chunk, size_t start, size_t _pkt_len);
        std::string respond();
        void        closeCGI();

        void        setCGI(std::string cgiPath);

        bool                isStarted() const;
        const std::string   &getRawResp() const;
        const std::string   &getBody() const;
        const std::map<std::string, std::string> &getHeaders() const;
        int   getStatus() const;
        std::string         buildRawHeader() const;

};

#endif
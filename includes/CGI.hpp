#ifndef _CGI_
#define _CGI_

#include <iostream>
#include <sstream>
#include <unistd.h>
#include "Request.hpp"
#include "Route.hpp"

class CGI
{
    private :
        const Request     *_request;
        const Route       *_route;
        std::string _query_string;
        std::string _request_method;
        std::string _path_info;
        std::string _content_length;
        std::string _content_type;

        CGI(CGI const &a);
        CGI  &operator=(CGI const &a);

        void getPathInfo();
        void getQueryString();
        void getContentLength();
        void getContentType();
        void getRequestMethod();

    public :
        ~CGI();
        CGI();

        void        prepare(Request const &req, Route &route);
        std::string forwardReq();
        char        **buildEnvFromAttr();
};

#endif
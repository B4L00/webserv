#ifndef _CONFIG_
#define _CONFIG_

# include <iostream>
# include <vector>
# include <map>
# include <set>
# include <fstream>
# include <sstream>
# include <arpa/inet.h>
# include <cstdlib>

# include "ServerConf.hpp"
# include "Route.hpp"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE    30720
#endif
#ifndef SPACES
# define SPACES         " \n\t\f\r\v"
#endif
#ifndef DEBUG
# define DEBUG  true
#endif

void    split(std::string &in, std::vector<std::string> &out, char sep);
void    readAllFile(std::ifstream &fs, std::string &out);

class   Config
{
    private :
        static std::map<std::string, long>  str2permmap;
        std::vector<ServerConf>             _servers;
        std::string                         _file;
        bool                                _isInit;
        std::set<std::pair<in_addr_t, in_port_t> > _listensIpPort;
        std::map<std::string, bool (Config::*)(std::vector<std::string>&, ServerConf&)>   serverUpdateFuncs;
        std::map<std::string, bool (Config::*)(std::vector<std::string>&, Route&)>   routeUpdateFuncs;

        void                parse();
        void                parseServer(size_t left, size_t right);
        void                updateServerFromDirective(std::vector<std::string> &dirs, ServerConf &conf);
        void                extractLocations(std::string &substr, ServerConf &conf);
        void                parseLocation(std::string &substr, size_t left, size_t right, Route &route);
        void                updateRouteFromDirective(std::vector<std::string> &dirs, Route &route);

        bool                listen(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_name(std::vector<std::string> &dirs, ServerConf &conf);
        bool                error_page(std::vector<std::string> &dirs, ServerConf &conf);
        bool                client_max_body_size(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_root(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_index(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_autoindex(std::vector<std::string> &dirs, ServerConf &conf);
        bool                server_allowed_methods(std::vector<std::string> &dirs, ServerConf &conf);

        bool                root(std::vector<std::string> &dirs, Route &conf);
        bool                rewrite(std::vector<std::string> &dirs, Route &conf);
        bool                allowed_methods(std::vector<std::string> &dirs, Route &conf);
        bool                autoindex(std::vector<std::string> &dirs, Route &conf);
        bool                dir_default(std::vector<std::string> &dirs, Route &conf);
        bool                cgi_extension(std::vector<std::string> &dirs, Route &conf);
        bool                save_path(std::vector<std::string> &dirs, Route &conf);

        Config();
        Config(Config const &a);
        Config  &operator=(Config const &a);

    public :
        Config(std::string config_path);
        ~Config();

        void    initConfig(std::string &config_path);
        const ServerConf    &getServerFromHostAndIPPort(std::string host, std::string ip, sockaddr_in addr) const;
        std::vector<ServerConf>   &getServers();
        static long         str2perm(std::string &method_str);
        static std::string  perm2str(long perm);
        const std::set<std::pair<in_addr_t, in_port_t> > &getListens();
};

#endif
#ifndef GSPC_NET_UTIL_HPP
#define GSPC_NET_UTIL_HPP

#include <string>

namespace gspc
{
  namespace net
  {
    size_t split_host_port ( std::string const & host_port
                           , std::string & host
                           , std::string & port
                           );
    void join_host_port ( std::string const & host
                        , std::string const & port
                        , std::string & host_port
                        );
  }
}

#endif

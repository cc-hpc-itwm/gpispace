#ifndef GSPC_NET_HANDLE_HPP
#define GSPC_NET_HANDLE_HPP

#include <gspc/net/server/service_handler.hpp>

namespace gspc
{
  namespace net
  {
    /**
       Register a service with the default service demux.
    */
    void handle ( std::string const &service
                , gspc::net::server::service_handler_t handler
                );
  }
}

#endif

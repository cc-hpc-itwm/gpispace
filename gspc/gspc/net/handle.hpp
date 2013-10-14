#ifndef GSPC_NET_HANDLE_HPP
#define GSPC_NET_HANDLE_HPP

#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace net
  {
    /**
       Register a service with the default service demux.
    */
    void handle ( std::string const &service
                , gspc::net::service::handler_t handler
                );
    void unhandle (std::string const &service);
  }
}

#endif

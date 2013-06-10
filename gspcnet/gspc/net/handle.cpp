#include "handle.hpp"

#include <gspc/net/server/default_service_demux.hpp>

namespace gspc
{
  namespace net
  {
    void handle ( std::string const &service
                , gspc::net::service::handler_t handler
                )
    {
      gspc::net::server::default_service_demux ().handle (service, handler);
    }

    void unhandle (std::string const &service)
    {
      gspc::net::server::default_service_demux ().unhandle (service);
    }
  }
}

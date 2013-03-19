#ifndef GSPC_NET_HANDLER_STRIP_PREFIX_HPP
#define GSPC_NET_HANDLER_STRIP_PREFIX_HPP

#include <string>
#include <gspc/net/server/service_handler.hpp>

namespace gspc
{
  namespace net
  {
    namespace handler
    {
      struct strip_prefix
      {
        strip_prefix ( std::string const &prefix
                     , gspc::net::server::service_handler_t next
                     );

        void operator () ( std::string const &
                         , frame const &rqst
                         , frame & rply
                         );
      private:
        std::string m_prefix;
        gspc::net::server::service_handler_t m_next;
      };
    }
  }
}

#endif

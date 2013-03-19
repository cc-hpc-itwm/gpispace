#ifndef GSPC_NET_HANDLER_STRIP_PREFIX_HPP
#define GSPC_NET_HANDLER_STRIP_PREFIX_HPP

#include <string>
#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace net
  {
    namespace service
    {
      struct strip_prefix
      {
        strip_prefix ( std::string const &prefix
                     , handler_t next
                     );

        void operator () ( std::string const &
                         , frame const &rqst
                         , frame & rply
                         );
      private:
        std::string m_prefix;
        handler_t m_next;
      };
    }
  }
}

#endif

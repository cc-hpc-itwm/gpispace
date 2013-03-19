#ifndef GSPC_NET_HANDLER_ECHO_HPP
#define GSPC_NET_HANDLER_ECHO_HPP

#include <string>
#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace handler
    {
      struct echo
      {
        void operator () ( std::string const &
                         , frame const &rqst
                         , frame & rply
                         );
      };
    }
  }
}

#endif

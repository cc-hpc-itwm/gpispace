#ifndef GSPC_NET_SERVER_SERVICE_HANDLER_HPP
#define GSPC_NET_SERVER_SERVICE_HANDLER_HPP

#include <string>
#include <boost/function.hpp>
#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      typedef boost::function<void ( std::string const &
                                   , frame const &
                                   , frame &
                                   )
                             > service_handler_t;
    }
  }
}

#endif

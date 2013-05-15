#ifndef GSPC_NET_SERVER_SERVICE_HANDLER_HPP
#define GSPC_NET_SERVER_SERVICE_HANDLER_HPP

#include <string>
#include <boost/function.hpp>
#include <gspc/net/frame.hpp>
#include <gspc/net/user_fwd.hpp>
#include <gspc/net/user.hpp>

namespace gspc
{
  namespace net
  {
    namespace service
    {
      typedef boost::function<void ( std::string const &
                                   , frame const &
                                   , user_ptr user
                                   )
                             > handler_t;
    }
  }
}

#endif

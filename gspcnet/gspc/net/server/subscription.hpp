#ifndef GSPC_NET_SERVER_SUBSCRIPTION_HPP
#define GSPC_NET_SERVER_SUBSCRIPTION_HPP

#include <string>

#include <gspc/net/user_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      struct subscription_t
      {
        std::string id;
        user_ptr    user;
        std::string destination;
      };
      typedef subscription_t* subscription_ptr;
    }
  }
}

#endif

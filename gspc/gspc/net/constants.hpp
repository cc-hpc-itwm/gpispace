#ifndef GSPC_NET_CONSTANTS_HPP
#define GSPC_NET_CONSTANTS_HPP

#include <cstddef>

namespace gspc
{
  namespace net
  {
    struct constants
    {
      static std::size_t MAX_LOST_HEARTBEATS ();
      static std::size_t CONNECT_TIMEOUT ();
    };
  }
}

#endif

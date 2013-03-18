#ifndef GSPC_NET_SERVER_USER_HPP_HPP
#define GSPC_NET_SERVER_USER_HPP_HPP

#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class user_t
      {
      public:
        virtual ~user_t () {}

        virtual int deliver (frame const &f) = 0;
      };
    }
  }
}

#endif

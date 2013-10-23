#ifndef GSPC_NET_USER_HPP
#define GSPC_NET_USER_HPP

#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    class user_t
    {
    public:
      virtual ~user_t () {}

      virtual int deliver (frame const &f) = 0;

      virtual size_t id () const = 0;
    };
  }
}

#endif

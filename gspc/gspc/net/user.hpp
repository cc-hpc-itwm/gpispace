#ifndef GSPC_NET_USER_HPP
#define GSPC_NET_USER_HPP

#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/heartbeat_info.hpp>

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

      virtual void set_heartbeat_info (heartbeat_info_t const &) = 0;
    };
  }
}

#endif

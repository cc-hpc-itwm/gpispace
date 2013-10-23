#ifndef GSPC_NET_FRAME_HANDLER_HPP
#define GSPC_NET_FRAME_HANDLER_HPP

#include <boost/system/error_code.hpp>
#include <gspc/net/user_fwd.hpp>
#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    class frame_handler_t
    {
    public:
      virtual ~frame_handler_t () {}

      virtual int handle_frame (user_ptr, frame const &) = 0;
      virtual int handle_error (user_ptr, boost::system::error_code const &) = 0;
    };
  }
}

#endif

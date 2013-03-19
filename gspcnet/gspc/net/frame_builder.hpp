#ifndef GSPC_NET_FRAME_BUILDER_HPP
#define GSPC_NET_FRAME_BUILDER_HPP

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace make
    {
      frame error_frame (int ec, const char *message);
      frame receipt (frame::value_type const &id);
    }
  }
}

#endif

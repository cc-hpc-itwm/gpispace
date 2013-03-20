#ifndef GSPC_NET_FRAME_BUILDER_HPP
#define GSPC_NET_FRAME_BUILDER_HPP

#include <gspc/net/frame.hpp>
#include <gspc/net/header.hpp>

namespace gspc
{
  namespace net
  {
    namespace make
    {
      frame error_frame (int ec, const char *message);
      frame receipt_frame (gspc::net::header::receipt const &);
    }
  }
}

#endif

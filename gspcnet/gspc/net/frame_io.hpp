#ifndef GSPC_NET_FRAME_IO_HPP
#define GSPC_NET_FRAME_IO_HPP

#include <iosfwd>

#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    std::ostream & operator << (std::ostream & os, frame const & f);
  }
}

#endif

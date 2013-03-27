#ifndef GSPC_NET_FRAME_BUFFER_HPP
#define GSPC_NET_FRAME_BUFFER_HPP

#include <vector>
#include <boost/asio/buffer.hpp>

#include <gspc/net/frame_fwd.hpp>

namespace gspc
{
  namespace net
  {
    std::vector<boost::asio::const_buffer> frame_to_buffers (frame const &f);
  }
}

#endif

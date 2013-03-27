#include "frame_buffer.hpp"

#include "frame.hpp"

namespace gspc
{
  namespace net
  {
    std::vector<boost::asio::const_buffer> frame_to_buffers (frame const &f)
    {
      std::vector<boost::asio::const_buffer> buffers;
      buffers.push_back
        (boost::asio::buffer (f.to_string ()));
      return buffers;
    }
  }
}

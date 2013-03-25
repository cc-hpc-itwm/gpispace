#ifndef GSPC_NET_LIMITS_HPP
#define GSPC_NET_LIMITS_HPP

#include <cstddef>

namespace gspc
{
  namespace net
  {
    namespace limits
    {
      std::size_t max_command_length ();
      std::size_t max_header_key_length ();
      std::size_t max_header_value_length ();
    }
  }
}

#endif

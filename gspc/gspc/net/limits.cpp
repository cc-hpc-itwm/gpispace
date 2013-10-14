#include "limits.hpp"

namespace gspc
{
  namespace net
  {
    namespace limits
    {
      std::size_t max_command_length ()
      {
        return 64;
      }

      std::size_t max_header_key_length ()
      {
        return 1024;
      }

      std::size_t max_header_value_length ()
      {
        return 4096;
      }

      std::size_t max_pending_frames_per_connection ()
      {
        return 8192u;
      }
    }
  }
}

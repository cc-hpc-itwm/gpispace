#include "frame_util.hpp"

namespace gspc
{
  namespace net
  {
    bool is_heartbeat (frame const &f)
    {
      return f.get_command () == "";
    }
  }
}

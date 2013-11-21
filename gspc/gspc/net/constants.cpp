#include "constants.hpp"

namespace gspc
{
  namespace net
  {
    std::size_t constants::MAX_LOST_HEARTBEATS ()
    {
      return 3;
    }

    std::size_t constants::CONNECT_TIMEOUT ()
    {
      return 30*1000;
    };
  }
}

#include "default_service_demux.hpp"

#include <gspc/net/server/service_demux.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      service_demux_t & default_service_demux ()
      {
        static service_demux_t demux;
        return demux;
      }
    }
  }
}

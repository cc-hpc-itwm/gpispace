#include "default_queue_manager.hpp"

#include <gspc/net/server/default_service_demux.hpp>
#include <gspc/net/server/queue_manager.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      queue_manager_t & default_queue_manager ()
      {
        static queue_manager_t qmgr (default_service_demux ());
        return qmgr;
      }
    }
  }
}

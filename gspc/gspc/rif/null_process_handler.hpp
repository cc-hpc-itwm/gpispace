#ifndef GSPC_RIF_NULL_PROCESS_HANDLER_HPP
#define GSPC_RIF_NULL_PROCESS_HANDLER_HPP

#include <gspc/rif/process_handler.hpp>

namespace gspc
{
  namespace rif
  {
    class null_process_handler_t : public process_handler_t
    {
    public:
      static null_process_handler_t *instance ();

      void onStateChange (proc_t p, process_state_t state);
    };
  }
}

#endif

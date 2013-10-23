#ifndef GSPC_RIF_PROCESS_HANDLER_HPP
#define GSPC_RIF_PROCESS_HANDLER_HPP

#include <gspc/rif/types.hpp>
#include <gspc/rif/process_state.hpp>

namespace gspc
{
  namespace rif
  {
    class process_handler_t
    {
    public:
      virtual ~process_handler_t () {}

      virtual void onStateChange (proc_t p, process_state_t state) = 0;
    };
  }
}

#endif

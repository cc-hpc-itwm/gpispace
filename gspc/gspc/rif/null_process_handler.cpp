#include "null_process_handler.hpp"

namespace gspc
{
  namespace rif
  {
    null_process_handler_t *null_process_handler_t::instance ()
    {
      static null_process_handler_t _instance;
      return &_instance;
    }

    void null_process_handler_t::onStateChange (proc_t, process_state_t)
    {}
  }
}

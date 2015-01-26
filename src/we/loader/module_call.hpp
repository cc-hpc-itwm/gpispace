#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/activity.hpp>
#include <we/type/module_call.hpp>

namespace we
{
  namespace loader
  {
    void module_call ( we::loader::loader& loader
                     , drts::worker::context* context
                     , we::type::activity_t& act
                     , const we::type::module_call_t& module_call
                     );
  }
}

#endif

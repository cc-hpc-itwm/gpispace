#include <we/loader/module_call.hpp>

#include <we/type/id.hpp>
#include <we/type/port.hpp>

//! \todo remove, needed to make a complete type
#include <we/type/net.hpp>

namespace we
{
  namespace
  {
    expr::eval::context input (we::type::activity_t const& activity)
    {
      expr::eval::context context;

      for (auto const& token_on_port : activity.input())
      {
        context.bind_ref
          ( activity.transition().ports_input().at (token_on_port.second).name()
          , token_on_port.first
          );
      }

      return context;
    }
  }

  namespace loader
  {
    void module_call ( we::loader::loader& loader
                     , drts::worker::context* context
                     , we::type::activity_t& act
                     , const we::type::module_call_t& module_call
                     )
    {
      typedef std::pair< we::port_id_type
                       , we::type::port_t
                       > port_by_id_type;

      expr::eval::context out;

      loader[module_call.module()].call
        (module_call.function(), context, input (act), out);

      for (const port_by_id_type& port_by_id : act.transition().ports_output())
      {
        const we::port_id_type& port_id (port_by_id.first);
        const we::type::port_t& port (port_by_id.second);

        act.add_output (port_id, out.value (port.name()));
      }
    }
  }
}

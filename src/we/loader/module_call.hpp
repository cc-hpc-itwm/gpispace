#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/module_call.hpp>
#include <we/type/port.hpp>

#include <boost/foreach.hpp>

namespace we
{
  namespace loader
  {
    static void module_call ( we::loader::loader& loader
                            , gspc::drts::context* context
                            , we::mgmt::type::activity_t& act
                            , const we::type::module_call_t& module_call
                            )
    {
      typedef std::pair< pnet::type::value::value_type
                       , petri_net::port_id_type
                       > token_on_port_type;
      typedef std::pair< petri_net::port_id_type
                       , we::type::port_t
                       > port_by_id_type;

      expr::eval::context in;
      expr::eval::context out;

      BOOST_FOREACH (const token_on_port_type& token_on_port, act.input())
      {
        in.bind_ref ( act.transition().ports().at (token_on_port.second).name()
                    , token_on_port.first
                    );
      }

      loader[module_call.module()].call
        (module_call.function(), context, in, out);

      BOOST_FOREACH ( const port_by_id_type& port_by_id
                    , act.transition().ports()
                    )
        {
          const petri_net::port_id_type& port_id (port_by_id.first);
          const we::type::port_t& port (port_by_id.second);

          if (port.is_output())
          {
            act.add_output (port_id, out.value (port.name()));
          }
        }
    }
  }
}

#endif

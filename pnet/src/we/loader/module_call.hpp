#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/types.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/type/module_call.hpp>
#include <we/type/id.hpp>
#include <we/type/port.hpp>

#include <we/require_type.hpp>

#include <boost/foreach.hpp>

namespace module
{
  static void call (we::loader::loader & loader, we::mgmt::type::activity_t & act, const we::type::module_call_t & module_call)
  {
    // construct context
    typedef we::loader::input_t context_t;
    typedef we::mgmt::type::activity_t::output_t output_t;
    typedef we::type::port_t port_t;
    typedef we::type::transition_t::const_iterator port_iterator;

    context_t context;

    typedef std::pair< pnet::type::value::value_type
                     , petri_net::port_id_type
                     > tp_type;

    BOOST_FOREACH (const tp_type& tp, act.input())
    {
      context.bind_ref ( act.transition().name_of_port (tp.second)
                       , tp.first
                       );
    }

    we::loader::output_t mod_output;

    loader[module_call.module()] (module_call.function(), context, mod_output);

    typedef std::pair<std::string, pnet::type::value::value_type> kv_type;

    BOOST_FOREACH ( const we::type::transition_t::port_map_t::value_type& ip
                  , act.transition().ports()
                  )
    {
      const petri_net::port_id_type& port_id (ip.first);
      const we::type::port_t& port (ip.second);

      if (port.is_output())
      {
        act.add_output ( output_t::value_type
                         (pnet::require_type ( mod_output.value (port.name())
                                             , port.signature()
                                             , port.name()
                                             )
                         , port_id
                         )
                       );
      }
    }
  }
}

#endif

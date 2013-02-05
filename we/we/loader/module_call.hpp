#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/types.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/type/module_call.hpp>
#include <we/type/id.hpp>
#include <we/type/port.hpp>
#include <we/type/value/require_type.hpp>

#include <we/mgmt/type/activity.hpp>

#include <boost/foreach.hpp>

namespace module
{
  static void call (we::loader::loader & loader, we::mgmt::type::activity_t & act, const we::type::module_call_t & module_call)
  {
    // construct context
    typedef we::loader::input_t context_t;
    typedef we::mgmt::type::activity_t::input_t input_t;
    typedef we::mgmt::type::activity_t::output_t output_t;
    typedef we::type::port_t port_t;
    typedef we::type::transition_t::const_iterator port_iterator;

    context_t context;

    typedef std::pair<value::type, petri_net::port_id_type> tp_type;

    BOOST_FOREACH (const tp_type& tp, act.input())
    {
      context.bind ( act.transition().name_of_port (tp.second)
                   , tp.first
                   );
    }

    typedef we::loader::output_t mod_output_t;

    mod_output_t mod_output;

    loader[module_call.module()] (module_call.function(), context, mod_output);

    typedef std::pair<std::string, value::type> kv_type;

    BOOST_FOREACH (const kv_type& kv, mod_output.values())
    {
      try
      {
        const petri_net::port_id_type& port_id
          (act.transition().output_port_by_name (kv.first));

        const port_t & port =
          act.transition().get_port (port_id);

        act.add_output
          ( output_t::value_type
            ( value::require_type ( port.name()
                                  , port.signature()
                                  , kv.second
                                  )
            , port_id
            )
          );
      }
      catch (const std::exception & e)
      {
        std::cout << "During collect output: " << e.what() << std::endl;

        throw;
      }
    }
  }
}

#endif

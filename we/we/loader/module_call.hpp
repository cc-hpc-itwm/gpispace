#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/types.hpp>

#include <we/type/module_call.hpp>
#include <we/type/id.hpp>
#include <we/type/port.hpp>

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
    for ( input_t::const_iterator top (act.input().begin())
        ; top != act.input().end()
        ; ++top
        )
    {
      const token::type token = top->first;
      const petri_net::port_id_type port_id = top->second;

      we::loader::put
        (context, act.transition().name_of_port (port_id), token.value);
    }

    typedef we::loader::output_t mod_output_t;

    mod_output_t mod_output;

    loader[module_call.module()] (module_call.function(), context, mod_output);

    for ( mod_output_t::const_iterator ton (mod_output.begin())
        ; ton != mod_output.end()
        ; ++ton
        )
    {
      try
      {
        const petri_net::port_id_type& port_id
          (act.transition().output_port_by_name (ton->first));

        const port_t & port =
          act.transition().get_port (port_id);

        act.add_output
          ( output_t::value_type
            ( token::type ( port.name()
                          , port.signature()
                          , ton->second
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

#ifndef SDPA_DAEMON_NRE_MODULE_CALL_HPP
#define SDPA_DAEMON_NRE_MODULE_CALL_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/types.hpp>

namespace module
{
  static void call (we::loader::loader & loader, we::activity_t & act, const we::transition_t::mod_type & module_call)
  {
    we::mgmt::type::detail::printer<we::activity_t, std::ostream> printer (act, std::cout);

    // construct context
    typedef we::loader::input_t context_t;
    typedef we::activity_t::input_t input_t;
    typedef we::activity_t::output_t output_t;
    typedef we::activity_t::transition_type::port_id_t port_id_t;
    typedef we::activity_t::transition_type::port_t port_t;
    typedef we::activity_t::transition_type::const_iterator port_iterator;

    context_t context;
    for ( input_t::const_iterator top (act.input().begin())
        ; top != act.input().end()
        ; ++top
        )
    {
      const token::type token   = top->first;
      const port_id_t  port_id = top->second;

      we::loader::put
        ( context
        , we::type::detail::translate_port_to_name ( act.transition()
                                                   , port_id
                                                   )
        , token.value
        );
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
        const port_id_t port_id =
          we::type::detail::translate_name_to_output_port ( act.transition()
                                                          , ton->first
                                                          );

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

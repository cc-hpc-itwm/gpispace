#ifndef WE_UTIL_PUTGET_HPP
#define WE_UTIL_PUTGET_HPP 1

#include <list>

#include <we/we.hpp>

namespace we
{
  namespace util
  {
    namespace token
    {
      typedef ::value::type type;
      typedef std::list<type> list_t;

      we::activity_t & put ( we::activity_t & act
                           , std::string const & port
                           , type const & value
                           )
      {
        typedef we::activity_t::transition_type::port_id_t port_id_t;
        const port_id_t pid (act.transition().input_port_by_name(port));

        act.add_input
          (we::input_t::value_type( we::token_t ( port
                                                , act.transition().get_port(pid).signature()
                                                , value
                                                )
                                  , pid
                                  )
          );
        return act;
      }

      list_t get ( we::activity_t const & act
                 , std::string const & port
                 )
      {
        typedef we::activity_t::transition_type::port_id_t port_id_t;
        typedef we::activity_t::output_t output_t;

        list_t tokens;
        const port_id_t port_id (act.transition().output_port_by_name(port));
        for ( output_t::const_iterator out(act.output().begin())
            ; out != act.output().end()
            ; ++out
            )
        {
          if (out->second == port_id)
            tokens.push_back (out->first.value);
        }

        return tokens;
      }
    }
  }
}

#endif


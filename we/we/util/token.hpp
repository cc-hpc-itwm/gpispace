#ifndef WE_UTIL_PUTGET_HPP
#define WE_UTIL_PUTGET_HPP 1

#include <list>

#include <we/type/token.hpp>
#include <we/type/id.hpp>

#include <we/we.hpp>

namespace we
{
  namespace util
  {
    namespace token
    {
      typedef ::value::type type;
      typedef std::list<type> list_t;
      typedef std::map<std::string, list_t> marking_t;

      we::activity_t & put ( we::activity_t & act
                           , std::string const & port
                           , type const & value
                           )
      {
        const petri_net::rid_t pid (act.transition().input_port_by_name(port));

        act.add_input
          (we::input_t::value_type( ::token::type ( port
                                                  , act.transition().get_port(pid).signature()
                                                  , value
                                                  )
                                  , pid
                                  )
          );
        return act;
      }

      /*

        TODO: make this work.

        see we/we.hpp for the to_value template

      template <typename T>
      we::activity_t & put ( we::activity_t & act
                           , std::string const & port
                           , T const & val
                           )
      {
        using namespace we::util::token;
        return put( act, port, to_value(val) );
      }
      */

      // TODO:
      // try the following:
      //    generic templetized function that may be overridden: to_value/from_value
      //    so that put/get can take any structs
      // see: hash_value

      list_t get ( we::activity_t const & act
                 , std::string const & port
                 )
      {
        typedef we::activity_t::output_t output_t;

        list_t tokens;
        const petri_net::rid_t port_id (act.transition().output_port_by_name(port));
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

      marking_t get_input (we::activity_t const & act)
      {
        typedef we::activity_t::input_t input_t;

        marking_t m;
        for ( input_t::const_iterator in(act.input().begin()), end(act.input().end())
            ; in != end
            ; ++in
            )
        {
          const std::string port_name (act.transition().get_port(in->second).name());
          m[port_name].push_back (in->first.value);
        }

        return m;
      }

      marking_t get_output (we::activity_t const & act)
      {
        typedef we::activity_t::output_t output_t;

        marking_t m;
        for ( output_t::const_iterator out(act.output().begin()), end(act.output().end())
            ; out != end
            ; ++out
            )
        {
          const std::string port_name (act.transition().get_port(out->second).name());
          m[port_name].push_back (out->first.value);
        }

        return m;
      }
    }
  }
}

#endif


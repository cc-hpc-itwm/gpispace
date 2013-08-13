#ifndef WE_UTIL_PUTGET_HPP
#define WE_UTIL_PUTGET_HPP 1

#include <list>

#include <we/type/value.hpp>
#include <we/type/id.hpp>

#include <we2/require_type.hpp>
#include <we2/type/value.hpp>
#include <we2/type/compat.hpp>
#include <we2/type/compat.sig.hpp>

#include <we/mgmt/type/activity.hpp>

namespace we
{
  namespace util
  {
    namespace token
    {
      typedef ::value::type type;
      typedef std::list<pnet::type::value::value_type> list_t;
      typedef std::map<std::string, list_t> marking_t;

      mgmt::type::activity_t & put ( mgmt::type::activity_t & act
                                   , std::string const & port
                                   , type const & value
                                   )
      {
        const ::petri_net::port_id_type pid
          (act.transition().input_port_by_name (port));

        const ::signature::type port_signature
          (act.transition().get_port (pid).signature());

        act.add_input ( mgmt::type::activity_t::input_t::value_type
                        ( pnet::require_type_relaxed
                          ( pnet::type::compat::COMPAT (value)
                          , pnet::type::compat::COMPAT (port_signature)
                          , port
                          )
                        , pid
                        )
                      );
        return act;
      }
      mgmt::type::activity_t & put2 ( mgmt::type::activity_t & act
                                    , std::string const & port
                                    , pnet::type::value::value_type const & value
                                    )
      {
        const ::petri_net::port_id_type pid
          (act.transition().input_port_by_name (port));

        const ::signature::type port_signature
          (act.transition().get_port (pid).signature());

        act.add_input ( mgmt::type::activity_t::input_t::value_type
                        ( pnet::require_type_relaxed
                          ( value
                          , pnet::type::compat::COMPAT (port_signature)
                          , port
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
      mgmt::type::activity_t & put ( mgmt::type::activity_t & act
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

      list_t get ( we::mgmt::type::activity_t const & act
                 , std::string const & port
                 )
      {
        typedef we::mgmt::type::activity_t::output_t output_t;

        list_t tokens;
        const petri_net::port_id_type port_id (act.transition().output_port_by_name(port));
        for ( output_t::const_iterator out(act.output().begin())
            ; out != act.output().end()
            ; ++out
            )
        {
          if (out->second == port_id)
            tokens.push_back (out->first);
        }

        return tokens;
      }

      marking_t get_input (we::mgmt::type::activity_t const & act)
      {
        typedef we::mgmt::type::activity_t::input_t input_t;

        marking_t m;
        for ( input_t::const_iterator in(act.input().begin()), end(act.input().end())
            ; in != end
            ; ++in
            )
        {
          const std::string port_name (act.transition().get_port(in->second).name());
          m[port_name].push_back (in->first);
        }

        return m;
      }

      marking_t get_output (we::mgmt::type::activity_t const & act)
      {
        typedef we::mgmt::type::activity_t::output_t output_t;

        marking_t m;
        for ( output_t::const_iterator out(act.output().begin()), end(act.output().end())
            ; out != end
            ; ++out
            )
        {
          const std::string port_name (act.transition().get_port(out->second).name());
          m[port_name].push_back (out->first);
        }

        return m;
      }
    }
  }
}

#endif

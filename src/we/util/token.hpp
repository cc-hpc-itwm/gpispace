#ifndef WE_UTIL_PUTGET_HPP
#define WE_UTIL_PUTGET_HPP 1

#include <list>

#include <we/type/id.hpp>

#include <we/type/value.hpp>

#include <we/mgmt/type/activity.hpp>

namespace we
{
  namespace util
  {
    namespace token
    {
      void put ( mgmt::type::activity_t & act
               , std::string const & port
               , pnet::type::value::value_type const & value
               )
      {
        const ::petri_net::port_id_type pid
          (act.transition().input_port_by_name (port));

        const pnet::type::signature::signature_type port_signature
          (act.transition().get_port (pid).signature());

        act.add_input ( mgmt::type::activity_t::input_t::value_type
                        (value, pid)
                      );
      }
    }
  }
}

#endif

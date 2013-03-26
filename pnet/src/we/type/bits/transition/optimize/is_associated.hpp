// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_IS_ASSOCIATED_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_IS_ASSOCIATED_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>
#include <we/type/port.hpp>

namespace we { namespace type {
    namespace optimize
    {
      inline bool is_associated ( const transition_t & trans
                                , const petri_net::place_id_type & pid
                                , we::type::port_t & port
                                )
      {
        try
          {
            port = trans.get_port_by_associated_pid (pid);

            return true;
          }
        catch (const exception::not_connected<petri_net::place_id_type> &)
          {
            return false;
          }
      }

      inline bool is_associated ( const transition_t & trans
                                , const petri_net::place_id_type & pid
                                )
      {
        we::type::port_t port;

        return is_associated (trans, pid, port);
      }
    }
  }
}

#endif

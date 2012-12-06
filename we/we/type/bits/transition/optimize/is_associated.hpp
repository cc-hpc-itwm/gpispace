// mirko.rahn@itwm.fraunhofer.de

#ifndef WE_TYPE_BITS_TRANSITION_OPTIMIZE_IS_ASSOCIATED_HPP
#define WE_TYPE_BITS_TRANSITION_OPTIMIZE_IS_ASSOCIATED_HPP 1

#include <we/type/transition.hpp>
#include <we/type/id.hpp>

namespace we { namespace type {
    namespace optimize
    {
      template<typename E>
      inline bool is_associated ( const transition_t<E> & trans
                                , const petri_net::pid_t & pid
                                , typename transition_t<E>::port_t & port
                                )
      {
        try
          {
            port = trans.get_port_by_associated_pid (pid);

            return true;
          }
        catch (const exception::not_connected<petri_net::pid_t> &)
          {
            return false;
          }
      }

      template<typename E>
      inline bool is_associated ( const transition_t<E> & trans
                                , const petri_net::pid_t & pid
                                )
      {
        typename transition_t<E>::port_t port;

        return is_associated (trans, pid, port);
      }
    }
  }
}

#endif

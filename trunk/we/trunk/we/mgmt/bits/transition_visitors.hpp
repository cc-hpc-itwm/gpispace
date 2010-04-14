/*
 * =====================================================================================
 *
 *       Filename:  transition_visitors.hpp
 *
 *    Description:  implementation of visitors required by activity instances
 *
 *        Version:  1.0
 *        Created:  04/15/2010 12:31:04 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_TRANSITION_VISITORS_HPP
#define WE_MGMT_BITS_TRANSITION_VISITORS_HPP 1

#include <stdexcept>
#include <string>

#include <boost/variant.hpp>
#include <we/type/transition.hpp>

namespace we { namespace mgmt { namespace visitor {
  namespace exception
  {
    struct operation_not_supported
      : public std::runtime_error
    {
      operation_not_supported (const std::string & msg)
        : std::runtime_error (msg)
      { }
    };
  }

  class has_enabled 
    : public boost::static_visitor<bool>
  {
  public:
    template <typename Place, typename Trans, typename Edge, typename Token>
    bool operator () (const petri_net::net<Place, Trans, Edge, Token> & net)
    {
      return ! net.enabled_transitions().empty();
    }

    template <typename T>
    bool operator () (const T &)
    {
      throw exception::operation_not_supported ("has_enabled");
    }
  };
}}}

#endif

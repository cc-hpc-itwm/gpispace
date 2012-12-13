#ifndef WE_WE_HPP
#define WE_WE_HPP 1

// this header defines our default types for petri-nets

#include <boost/static_assert.hpp>

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>

namespace we
{
  /*
    TODO: make this work (in principle it  already works, but I guess one has to
    predefine the to_value function for all the literals.

    template <typename T>
    ::value::type to_value (T const &)
    {
      BOOST_STATIC_ASSERT ((T*)0 && false);
      return 0;
    }
  */

  using petri_net::connection_t;
  using petri_net::edge::PT;
  using petri_net::edge::PT_READ;
  using petri_net::edge::TP;

  typedef we::type::transition_t transition_t;
  typedef petri_net::net pnet_t;
  typedef we::mgmt::type::activity_t activity_t;
  typedef activity_t::input_t input_t;

  using we::type::PORT_IN;
  using we::type::PORT_OUT;
}

#endif

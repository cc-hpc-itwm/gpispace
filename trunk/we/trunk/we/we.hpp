#ifndef WE_WE_HPP
#define WE_WE_HPP 1

// this header defines our default types for petri-nets

#include <we/net.hpp>
#include <we/util/codec.hpp>
#include <we/type/transition.hpp>
#include <we/type/place.hpp>
#include <we/type/token.hpp>
#include <we/mgmt/type/activity.hpp>

namespace we
{
  using petri_net::connection_t;
  using petri_net::PT;
  using petri_net::PT_READ;
  using petri_net::TP;

  typedef place::type place_t;
  typedef token::type token_t;
  typedef unsigned int edge_t;
  typedef we::type::transition_t<place_t, edge_t, token_t> transition_t;
  typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;
  typedef we::mgmt::type::activity_t<transition_t, token_t> activity_t;
  typedef activity_t::input_t input_t;

  using we::type::PORT_IN;
  using we::type::PORT_OUT;
  using we::type::PORT_READ;
  using we::type::PORT_IN_OUT;
}

#endif

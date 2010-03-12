#include <iostream>
#include <sstream>

#include <stdint.h>
#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;

typedef petri_net::net<we::mgmt::detail::place_t, we::mgmt::detail::transition_t, we::mgmt::detail::edge_t, we::mgmt::detail::token_t> pnet_t;

template <typename Stream>
inline Stream & operator << (Stream & s, const pnet_t & n)
{
  for (typename pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
  	s << "[" << n.get_place (*p) << ":";

  	for (typename pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
  	  s << " " << *tp;

  	s << "]";
    }

  return s;
}

int main ()
{
	typedef uint64_t id_type;
//  typedef unsigned long id_type;
//  typedef unsigned int id_type;
//  typedef int id_type;
//  typedef std::string id_type;
  typedef we::mgmt::layer<basic_layer<id_type>, pnet_t> layer_t;
  typedef sdpa_daemon<layer_t> daemon_type;

  // instantiate layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.layer();

  for (std::size_t i (0); i < 1; ++i)
  {
	daemon_type::id_type id = daemon.gen_id();

	mgmt_layer.submit(id, "");
	mgmt_layer.suspend(id);
	mgmt_layer.resume(id);
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.cancel(id, "");
  }

  sleep(1); // not nice, but we cannot wait for a network to finish right now

  return EXIT_SUCCESS;
}

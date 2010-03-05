#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;
using namespace test;

int main ()
{
  typedef unsigned long id_type;
//  typedef unsigned int id_type;
//  typedef int id_type;
//  typedef std::string id_type;
  typedef petri_net::net<we::mgmt::detail::place_t, we::mgmt::detail::transition_t, we::mgmt::detail::edge_t, we::mgmt::detail::token_t> pnet_t;
  typedef we::mgmt::layer<basic_layer<id_type>, pnet_t> layer_t;
  typedef sdpa_daemon<layer_t> daemon_type;

  // instantiate layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.layer();

  for (std::size_t i (0); i < 10; ++i)
  {
	daemon_type::id_type id = daemon.gen_id();

	mgmt_layer.submit(id, "");
	mgmt_layer.cancel(id, "");
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.suspend(id);
	mgmt_layer.resume(id);
  }

  return EXIT_SUCCESS;
}

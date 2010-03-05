#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;

int main ()
{
  typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;
  typedef we::mgmt::layer<basic_layer<>, pnet_t> layer_t;
  typedef sdpa_daemon<layer_t> daemon_type;

  // instantiate layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.mgmt_layer;

  for (std::size_t id (0); id < 10; ++id)
  {
	mgmt_layer.submit(id, "");
	mgmt_layer.cancel(id, "");
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.suspend(id);
	mgmt_layer.resume(id);
  }

  return EXIT_SUCCESS;
}

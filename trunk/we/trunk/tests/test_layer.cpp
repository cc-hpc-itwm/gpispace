#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;

int main ()
{
  typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;
  typedef we::mgmt::layer<basic_layer<std::string>, pnet_t> layer_t;
  typedef sdpa_daemon<layer_t> daemon_type;

  // instantiate layer
  daemon_type daemon;
  daemon_type::layer_type & mgmt_layer = daemon.mgmt_layer;

  for (std::size_t id (0); id < 10; ++id)
  {
	mgmt_layer.submit(daemon.gen_id(), "");
	mgmt_layer.cancel(daemon.gen_id(), "");
	mgmt_layer.failed(daemon.gen_id(), "");
	mgmt_layer.finished(daemon.gen_id(), "");
	mgmt_layer.suspend(daemon.gen_id());
	mgmt_layer.resume(daemon.gen_id());
  }

  return EXIT_SUCCESS;
}

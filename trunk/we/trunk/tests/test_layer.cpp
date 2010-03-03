#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>
#include "test_layer.hpp"

using namespace we::mgmt;

int main ()
{
  typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;

  std::cout << "running layer test..." << std::endl;
  pnet_t net("test_layer");
  parser<pnet_t>::parse(net, ""); 

  std::cout << net << std::endl;

  std::cout << "#enabled=" << net.enabled_transitions().size() << std::endl;

  // instantiate layer
  typedef sdpa_daemon<pnet_t> daemon_type;
  daemon_type daemon;
  daemon_type::mgmt_layer_type & mgmt_layer = daemon.mgmt_layer;

  for (std::size_t i (0); i < 10; ++i)
  {
	daemon_type::mgmt_layer_type::id_type id = mgmt_layer.submit(i, "");
	mgmt_layer.cancel(id, "");
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.status(id);
  }

  return EXIT_SUCCESS;
}

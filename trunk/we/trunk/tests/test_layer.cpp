#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>

using namespace we::mgmt;

typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;

struct sdpa_exec
{

};

int main ()
{
  std::cout << "running layer test..." << std::endl;
  pnet_t net("test_layer");
  parse<pnet_t, NetTraits<pnet_t>, std::string>(net, ""); 

  std::cout << net << std::endl;

  std::cout << "#enabled=" << net.enabled_transitions().size() << std::endl;

  // instantiate layer
  typedef layer<sdpa_exec, pnet_t> pnet_mgmt_layer_t;
  pnet_mgmt_layer_t mgmt_layer;

  for (std::size_t i (0); i < 10; ++i)
  {
	pnet_mgmt_layer_t::id_type id = mgmt_layer.submit("");
	std::cout << "id=" << id << std::endl;
	mgmt_layer.cancel(id, "");
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.status(id);
  }

  return EXIT_SUCCESS;
}

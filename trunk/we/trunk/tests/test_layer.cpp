#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>

using namespace we::mgmt;

template <typename Net>
struct sdpa_daemon
{
  typedef Net net_type;
  typedef typename we::mgmt::layer<sdpa_daemon, net_type> mgmt_layer_type;

  sdpa_daemon()
	: mgmt_layer(*this)
  {}

  mgmt_layer_type mgmt_layer;
};

int main ()
{
  typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;

  std::cout << "running layer test..." << std::endl;
  pnet_t net("test_layer");
  parse<pnet_t, NetTraits<pnet_t>, std::string>(net, ""); 

  std::cout << net << std::endl;

  std::cout << "#enabled=" << net.enabled_transitions().size() << std::endl;

  // instantiate layer
  typedef sdpa_daemon<pnet_t> daemon_type;
  daemon_type daemon;
  daemon_type::mgmt_layer_type & mgmt_layer = daemon.mgmt_layer;

  for (std::size_t i (0); i < 10; ++i)
  {
	daemon_type::mgmt_layer_type::id_type id = mgmt_layer.submit("");
	std::cout << "id=" << id << std::endl;
	mgmt_layer.cancel(id, "");
	mgmt_layer.failed(id, "");
	mgmt_layer.finished(id, "");
	mgmt_layer.status(id);
  }

  return EXIT_SUCCESS;
}

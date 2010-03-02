#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>

using namespace we::mgmt;

typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;

int main ()
{
  std::cout << "running layer test..." << std::endl;
  pnet_t net("test_layer");
  parse<pnet_t, NetTraits<pnet_t>, std::string>(net, ""); 

  std::cout << net << std::endl;

  std::cout << "#enabled=" << net.enabled_transitions().size() << std::endl;

  return EXIT_SUCCESS;
}

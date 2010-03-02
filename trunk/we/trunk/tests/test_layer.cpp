#include <iostream>
#include <sstream>

#include <we/mgmt/layer.hpp>

using namespace we::mgmt;

typedef petri_net::net<detail::place_t, detail::transition_t, detail::edge_t, detail::token_t> pnet_t;

static std::ostream & operator << (std::ostream & s, const pnet_t & n)
{
  for (pnet_t::place_const_it p (n.places()); p.has_more(); ++p)
    {
      s << "[" << n.get_place (*p) << ":";

      for (pnet_t::token_place_it tp (n.get_token (*p)); tp.has_more(); ++tp)
        s << " " << *tp;

      s << "]";
    }

  return s;
}

int main ()
{
  std::cout << "running layer test..." << std::endl;
  pnet_t net("test_layer");
  parse<pnet_t, NetTraits<pnet_t>, std::string>(net, ""); 

  std::cout << net << std::endl;

  std::cout << "#enabled=" << net.enabled_transitions().size() << std::endl;

  return EXIT_SUCCESS;
}

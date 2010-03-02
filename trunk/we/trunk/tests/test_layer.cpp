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
  petri_net::pid_t pid_in = net.add_place(detail::place_t("in"));
  petri_net::pid_t pid_out = net.add_place(detail::place_t("out"));

  petri_net::tid_t tid_start (
	  net.add_transition(
		detail::transition_t("start", detail::transition_t::INTERNAL_SIMPLE)));
  petri_net::eid_t eid_start_in (
	  net.add_edge(detail::edge_t("in"), petri_net::connection_t (petri_net::PT, tid_start, pid_in)));
  petri_net::eid_t eid_start_out (
	  net.add_edge(detail::edge_t("out"), petri_net::connection_t (petri_net::TP, tid_start, pid_out)));

  detail::edge_t e("foo");

  std::cout << net << std::endl;

  std::cout 
	<< "in=" << pid_in << " out=" << pid_out
	<< " trans=" << tid_start
	<< " e_in" << eid_start_in << " e_out=" << eid_start_out
	<< std::endl;
  return EXIT_SUCCESS;
}

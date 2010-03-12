/*
 * =====================================================================================
 *
 *       Filename:  parser.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/25/2010 05:05:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_PARSER_HPP
#define WE_MGMT_PARSER_HPP 1

#include <we/mgmt/bits/types.hpp>

namespace we { namespace mgmt {
  template <typename Net>
  struct NetTraits
  {
	typedef typename petri_net::pid_t pid_t;
	typedef typename petri_net::tid_t tid_t;
	typedef typename petri_net::eid_t eid_t;

	typedef typename detail::transition_t transition_t;
	typedef typename detail::place_t place_t;
	typedef typename detail::token_t token_t;
	typedef typename detail::edge_t edge_t;
  };

  template <typename _Net, typename _Data = std::string, typename _Traits = NetTraits<_Net> >
  struct parser
  {
	typedef _Net net_type;
	typedef _Data data_type;
	typedef _Traits traits_type;

	typedef typename traits_type::pid_t pid_t;
	typedef typename traits_type::tid_t tid_t;
	typedef typename traits_type::transition_t transition_t;
	typedef typename traits_type::place_t place_t;
	typedef typename traits_type::edge_t edge_t;
	typedef typename traits_type::token_t token_t;

	static void parse (net_type & net, const data_type &)
	{
	  // map-reduce network with 3 parallel nodes

	  const std::size_t NUM_NODES=3;
	  const std::size_t NUM_TOKEN=1;

	  pid_t pid_in = net.add_place(place_t("in"));
	  pid_t pid_out = net.add_place(place_t("out"));

	  tid_t tid_map = net.add_transition (transition_t("map", transition_t::INTERNAL_SIMPLE));
	  tid_t tid_red = net.add_transition (transition_t("red", transition_t::INTERNAL_SIMPLE));

	  net.add_edge (edge_t("map"), petri_net::connection_t (petri_net::PT, tid_map, pid_in));
	  net.add_edge (edge_t("red"), petri_net::connection_t (petri_net::TP, tid_red, pid_out));

	  // create places and transitions
	  for (std::size_t n(0); n < NUM_NODES; ++n)
	  {
		const char wi_name[] = {'w', 'i', '_', '0'+n, 0}; // input place
		const char w_name[] = {'w', '_', '0'+n, 0}; // transition
		const char wo_name[] = {'w', 'o', '_', '0'+n, 0}; // output place

		const pid_t pid_wi = net.add_place(place_t(wi_name));
		const pid_t pid_wo = net.add_place(place_t(wo_name));
		const tid_t tid_w = net.add_transition( transition_t(w_name, transition_t::INTERNAL_SIMPLE));

		// connect map to work input
		net.add_edge (edge_t("map"), petri_net::connection_t (petri_net::TP, tid_map, pid_wi));

		// connect work in
		net.add_edge (edge_t("work in"), petri_net::connection_t (petri_net::PT, tid_w, pid_wi));

		// connect work out
		net.add_edge (edge_t("work out"), petri_net::connection_t (petri_net::TP, tid_w, pid_wo));

		// connect work output to red
		net.add_edge (edge_t("red"), petri_net::connection_t (petri_net::PT, tid_red, pid_wo));
	  }

	  for (std::size_t t (0); t < NUM_TOKEN; ++t)
	  {
		const char name[] = {'t', 'o', 'k', 'e', 'n', '-', '0' + t, 0};
		net.put_token(pid_in, token_t(name));
	  }
	}
  };
}}

#endif

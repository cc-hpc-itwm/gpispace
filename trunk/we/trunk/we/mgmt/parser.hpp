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

#include <we/util/show.hpp>
#include <we/util/warnings.hpp>
#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/bits/activity.hpp>

namespace we { namespace mgmt {
  template <typename Net>
  struct NetTraits
  {
	typedef typename petri_net::pid_t pid_t;
	typedef typename petri_net::tid_t tid_t;
	typedef typename petri_net::eid_t eid_t;

	typedef typename detail::place_t place_t;
	typedef typename detail::edge_t edge_t;
	typedef typename detail::token_t token_t;
	typedef typename detail::transition_t<place_t, edge_t, token_t> transition_t;
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

    // TODO parsing should return an "activity" style thing
    //      where this activity thing contains one of:
    //         - network
    //         - expression
    //         - module call
//	static void parse (net_type & net, const data_type &)
//	{
//	  // map-reduce network with 3 parallel nodes
//
//	  const std::size_t NUM_NODES=3;
//	  const std::size_t NUM_TOKEN=1;
//
//	  pid_t pid_in = net.add_place(place_t("in"));
//	  pid_t pid_out = net.add_place(place_t("out"));
//
//	  tid_t tid_map = net.add_transition (transition_t("map", transition_t::MODULE_CALL));
//	  tid_t tid_red = net.add_transition (transition_t("red", transition_t::MODULE_CALL));
//
//	  net.add_edge (edge_t("map"), petri_net::connection_t (petri_net::PT, tid_map, pid_in));
//	  net.add_edge (edge_t("red"), petri_net::connection_t (petri_net::TP, tid_red, pid_out));
//
//	  // create places and transitions
//	  for (std::size_t n(0); n < NUM_NODES; ++n)
//	  {
//		const pid_t pid_wi = net.add_place(place_t("wi_" + show(n)));
//		const pid_t pid_wo = net.add_place(place_t("wo_" + show(n)));
//		const tid_t tid_w = net.add_transition( transition_t("work_"+show(n), transition_t::MODULE_CALL));
//
//		// connect map to work input
//		net.add_edge (edge_t("map_" + show(n)), petri_net::connection_t (petri_net::TP, tid_map, pid_wi));
//
//		// connect work in
//		net.add_edge (edge_t("work_in_"+show(n)), petri_net::connection_t (petri_net::PT, tid_w, pid_wi));
//
//		// connect work out
//		net.add_edge (edge_t("work_out_" + show(n)), petri_net::connection_t (petri_net::TP, tid_w, pid_wo));
//
//		// connect work output to red
//		net.add_edge (edge_t("red_" + show(n)), petri_net::connection_t (petri_net::PT, tid_red, pid_wo));
//	  }
//
//	  for (std::size_t t (0); t < NUM_TOKEN; ++t)
//	  {
//		std::string name = "token-" + show(t);
//		net.put_token(pid_in, token_t(name));
//	  }
//	}

    template <typename Activity>
    static void parse( Activity & act, const data_type &)
    {
      // create the subnetwork
      net_type map_reduce_subnet ("map-reduce-subnet");
      pid_t mr_sn_inp = map_reduce_subnet.add_place (place_t("in"));
      pid_t mr_sn_out = map_reduce_subnet.add_place (place_t("out"));

      // create the atomic subtransitions
      {

      }

      // create the toplevel transition
      {
        net_type map_reduce ("map-reduce");

        pid_t mr_inp = map_reduce.add_place(place_t("in"));
        pid_t mr_out = map_reduce.add_place(place_t("out"));

        transition_t map_reduce_trans("map-reduce", map_reduce);
        map_reduce_trans.connect(mr_inp, mr_sn_inp);
        map_reduce_trans.connect(mr_out, mr_sn_out);
        map_reduce_trans.flags.internal = true;

        map_reduce.add_transition( map_reduce_trans );

        act.assign (map_reduce);
      }
    }
  };
}}

#endif

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
      const std::size_t NUM_NODES=3;
      const std::size_t NUM_TOKEN=3;

      // create the subnetwork
      net_type map_reduce_subnet ("map-reduce-subnet");

      // connector ports
      pid_t mr_sn_inp = map_reduce_subnet.add_place (place_t("in"));
      pid_t mr_sn_out = map_reduce_subnet.add_place (place_t("out"));

      // create the atomic transitions
      {
        typedef std::vector<pid_t> hull_t;
        hull_t hull_in;
        hull_in.reserve(NUM_NODES);

        hull_t hull_out;
        hull_out.reserve(NUM_NODES);

        // create some parallelism
        for (std::size_t n(0); n < NUM_NODES; ++n)
        {
          const pid_t pid_wi = map_reduce_subnet.add_place(place_t("wi_" + ::util::show(n)));
          hull_in.push_back (pid_wi);

          const pid_t pid_wo = map_reduce_subnet.add_place(place_t("wo_" + ::util::show(n)));
          hull_out.push_back (pid_wo);

          transition_t wrk_trans ("work"+::util::show(n), typename transition_t::mod_type("map_reduce", "work"));
          wrk_trans.connect_in (pid_wi, pid_t(0));
          wrk_trans.connect_out (pid_wo, pid_t(1));

          const tid_t tid_w = map_reduce_subnet.add_transition (wrk_trans);

          // connect work in
          map_reduce_subnet.add_edge (edge_t("work_in_"+::util::show(n)), petri_net::connection_t (petri_net::PT, tid_w, pid_wi));

          // connect work out
          map_reduce_subnet.add_edge (edge_t("work_out_" + ::util::show(n)), petri_net::connection_t (petri_net::TP, tid_w, pid_wo));
        }

        {
          transition_t map_trans ("map", typename transition_t::expr_type("$1 := (1); $2 := (2); $3 := (3);"));

          // emulate ports for now
          map_trans.connect_in ( mr_sn_inp, pid_t (0) ); // port_0

          size_t cnt(0);
          for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
          {
            map_trans.connect_out ( *i, pid_t (1 + (cnt++)) ); // port_1 .. port_N
          }
          tid_t tid_map = map_reduce_subnet.add_transition ( map_trans );
          map_reduce_subnet.add_edge (edge_t("map"), petri_net::connection_t (petri_net::PT, tid_map, mr_sn_inp));

          cnt = 0;
          for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
          {
            map_reduce_subnet.add_edge (edge_t("map_" + ::util::show(cnt)), petri_net::connection_t (petri_net::TP, tid_map, *i));
            cnt++;
          }
        }

        {
          transition_t red_trans ("red", typename transition_t::expr_type("$1 := (0);"));
          red_trans.connect_out (mr_sn_out, pid_t (NUM_NODES)); // port_1

          size_t cnt(0);
          for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
          {
            red_trans.connect_in ( *o, pid_t (0 + (cnt)) );
            cnt++;
          }
          tid_t tid_red = map_reduce_subnet.add_transition ( red_trans );
          map_reduce_subnet.add_edge (edge_t("red"), petri_net::connection_t (petri_net::TP, tid_red, mr_sn_out));

          cnt = 0;
          for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
          {
            map_reduce_subnet.add_edge (edge_t("red_" + ::util::show(cnt)), petri_net::connection_t (petri_net::PT, tid_red, *o));
            cnt++;
          }
        }
      }

      // create the toplevel transition
      {
        net_type map_reduce ("map-reduce-net");

        pid_t mr_inp = map_reduce.add_place(place_t("in"));
        pid_t mr_out = map_reduce.add_place(place_t("out"));

        transition_t map_reduce_sub_trans("map-reduce-subnet", map_reduce_subnet, true);
        map_reduce_sub_trans.connect_in  (mr_inp, mr_sn_inp);
        map_reduce_sub_trans.connect_out (mr_out, mr_sn_out);

        tid_t tid_sub = map_reduce.add_transition( map_reduce_sub_trans );
        map_reduce.add_edge (edge_t("i"), petri_net::connection_t (petri_net::PT, tid_sub, mr_inp));
        map_reduce.add_edge (edge_t("o"), petri_net::connection_t (petri_net::TP, tid_sub, mr_out));

        // dummy transition
        transition_t map_reduce_trans ("map-reduce", map_reduce, true);
        map_reduce_trans.connect_in (pid_t(0), mr_inp);
        map_reduce_trans.connect_out (pid_t(1), mr_out);
        act.assign (map_reduce_trans);

        // put some tokens on the input
        for (std::size_t t (0); t < NUM_TOKEN; ++t)
        {
          std::string name = "token-" + ::util::show(t);
          act.input().push_back( std::make_pair(token_t(name), pid_t(0))); //  ) );map_reduce.put_token(mr_inp, token_t(name));
        }
      }
    }
  };
}}

#endif

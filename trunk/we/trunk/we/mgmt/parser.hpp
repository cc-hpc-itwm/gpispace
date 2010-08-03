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

#include <fhg/util/show.hpp>

#include <we/type/transition.hpp>
#include <we/mgmt/bits/types.hpp>

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
	typedef typename we::type::transition_t<place_t, edge_t, token_t> transition_t;
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

    template <typename Activity>
    static Activity parse( const data_type &)
    {
      const std::size_t NUM_NODES=3;
      const std::size_t NUM_TOKEN=1;

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
          const pid_t pid_wi = map_reduce_subnet.add_place(place_t("wi_" + fhg::util::show(n)));
          hull_in.push_back (pid_wi);

          const pid_t pid_wo = map_reduce_subnet.add_place(place_t("wo_" + fhg::util::show(n)));
          hull_out.push_back (pid_wo);

          transition_t wrk_trans ("work"+fhg::util::show(n), typename transition_t::mod_type("map_reduce", "work"), "true");
          wrk_trans.add_ports()
            ("i", "long", we::type::PORT_IN)
            ("o", "long", we::type::PORT_OUT)
          ;
          wrk_trans.add_connections()
            (pid_wi, "i")
            ("o", pid_wo)
          ;

          const tid_t tid_w = map_reduce_subnet.add_transition (wrk_trans);

          // connect work in
          map_reduce_subnet.add_edge (edge_t("work_in_"+fhg::util::show(n)), petri_net::connection_t (petri_net::PT, tid_w, pid_wi));

          // connect work out
          map_reduce_subnet.add_edge (edge_t("work_out_" + fhg::util::show(n)), petri_net::connection_t (petri_net::TP, tid_w, pid_wo));
        }

        {
          transition_t map_trans ("map", typename transition_t::expr_type("${1} := ${0} + \"-1\"; ${2} := ${0} + \"-2\"; ${3} := ${0} + \"-3\";"), "true", true);
          map_trans.add_ports()
            ("i", "long", we::type::PORT_IN)
          ;
          map_trans.add_connections()
            (mr_sn_inp, "i")
          ;

          size_t cnt(0);
          for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
          {
            map_trans.add_ports() ("o"+fhg::util::show (cnt), "long", we::type::PORT_OUT);
            map_trans.add_connections() ("o"+fhg::util::show (cnt), *i);
            cnt++;
          }
          tid_t tid_map = map_reduce_subnet.add_transition ( map_trans );
          map_reduce_subnet.add_edge (edge_t("map"), petri_net::connection_t (petri_net::PT, tid_map, mr_sn_inp));

          cnt = 0;
          for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
          {
            map_reduce_subnet.add_edge (edge_t("map_" + fhg::util::show(cnt)), petri_net::connection_t (petri_net::TP, tid_map, *i));
            cnt++;
          }
        }

        {
          transition_t red_trans ("red", typename transition_t::expr_type("${1} := substr(${0}, len(\"token-0\"));"), "true", true);

          size_t cnt(0);
          for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
          {
            red_trans.add_ports() ("i" + fhg::util::show(cnt), "long", we::type::PORT_IN);
            red_trans.add_connections() (*o, "i" + fhg::util::show(cnt));
            cnt++;
          }
          red_trans.add_ports() ("o", "long", we::type::PORT_OUT);
          red_trans.add_connections() ("o", mr_sn_out);

          tid_t tid_red = map_reduce_subnet.add_transition ( red_trans );
          map_reduce_subnet.add_edge (edge_t("red"), petri_net::connection_t (petri_net::TP, tid_red, mr_sn_out));

          cnt = 0;
          for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
          {
            map_reduce_subnet.add_edge (edge_t("red_" + fhg::util::show(cnt)), petri_net::connection_t (petri_net::PT, tid_red, *o));
            cnt++;
          }
        }
      }

      // create the toplevel transition
      {
        net_type map_reduce ("map-reduce-net");

        pid_t mr_inp = map_reduce.add_place(place_t("in"));
        pid_t mr_out = map_reduce.add_place(place_t("out"));

        transition_t map_reduce_sub_trans("map-reduce-subnet", map_reduce_subnet, "true", true);
        map_reduce_sub_trans.add_ports ()
          ("i", "long", we::type::PORT_IN)  // TODO port_id must be: mr_sn_inp
          ("o", "long", we::type::PORT_OUT) // TODO port_id must be: mr_sn_out
        ;
        map_reduce_sub_trans.add_connections ()
          (mr_inp, "i")
          ("o", mr_out)
        ;

//        map_reduce_sub_trans.connect_in  (mr_inp, mr_sn_inp);
//        map_reduce_sub_trans.connect_out (mr_out, mr_sn_out);

        tid_t tid_sub = map_reduce.add_transition( map_reduce_sub_trans );
        map_reduce.add_edge (edge_t("i"), petri_net::connection_t (petri_net::PT, tid_sub, mr_inp));
        map_reduce.add_edge (edge_t("o"), petri_net::connection_t (petri_net::TP, tid_sub, mr_out));

        // dummy transition
        transition_t map_reduce_trans ("map-reduce", map_reduce, "true", true);
        map_reduce_trans.add_ports()
          ("i", "long", we::type::PORT_IN)  // TODO port_id must be mr_inp
          ("o", "long", we::type::PORT_OUT) // TODO port_id must be mr_out
        ;
//        map_reduce_trans.connect_in (pid_t(0), mr_inp);
//        map_reduce_trans.connect_out (pid_t(1), mr_out);

        Activity act (map_reduce_trans);

        // put some tokens on the input
        for (std::size_t t (0); t < NUM_TOKEN; ++t)
        {
          act.input().push_back (std::make_pair (token_t ("token-" + fhg::util::show(t)), map_reduce_trans.input_port_by_name ("i")));
        }

        return act;
      }
    }
  };
}}

#endif

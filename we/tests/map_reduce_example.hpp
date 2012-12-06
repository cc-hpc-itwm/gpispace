#ifndef WE_TESTS_MAP_REDUCE_EXAMPLE_HPP
#define WE_TESTS_MAP_REDUCE_EXAMPLE_HPP 1

namespace we {
  namespace tests {
    template <typename Activity>
    struct map_reduce
    {
      typedef typename Activity::transition_type transition_type;

      typedef typename transition_type::place_type place_type;
      typedef typename transition_type::edge_type edge_type;

      typedef typename transition_type::net_type net_type;
      typedef typename transition_type::mod_type mod_type;
      typedef typename transition_type::expr_type expr_type;

      typedef typename transition_type::pid_t pid_t;
      typedef typename net_type::tid_t tid_t;
      typedef typename transition_type::port_id_t port_id_t;

      static transition_type generate_inner ()
      {
        // In:  input (some token), num_nodes (number of nodes)
        // Out: output (result after map-reduce)
        net_type map_reduce_subnet ("map-reduce-subnet");

        // **************
        // *** places ***
        // **************

        pid_t pid_input  (map_reduce_subnet.add_place (place_t ("input", "long")));
        pid_t pid_num_nodes  (map_reduce_subnet.add_place (place_t ("num_nodes", "long")));

        pid_t pid_node_count (map_reduce_subnet.add_place (place_t ("node_count", "long")));

        pid_t pid_work_in (map_reduce_subnet.add_place (place_t ("work_in", "long")));
        pid_t pid_work_out (map_reduce_subnet.add_place (place_t ("work_out", "long")));
        pid_t pid_output (map_reduce_subnet.add_place (place_t ("output", "long")));

        pid_t pid_reduce_state (map_reduce_subnet.add_place (place_t ("reduce_state", "long")));

        signature::structured_t sig_reduce_counter;
        sig_reduce_counter["max"] = "long";
        sig_reduce_counter["i"] = "long";

        pid_t pid_reduce_counter
          ( map_reduce_subnet.add_place
            ( place_t ( "reduce_counter", sig_reduce_counter )));

        // *******************
        // *** transitions ***
        // *******************

        // init
        transition_type trans_init
          ( "init"
          , expr_type
            ( "${reduce_counter.max} := ${nodes};"
              "${reduce_counter.i}   := 0;"
            )
          , "true"
          , true
          );
        trans_init.add_port
          ("nodes",          "long",             we::type::PORT_IN);
        trans_init.add_port
          ("reduce_counter", sig_reduce_counter, we::type::PORT_OUT);
        trans_init.add_connections ()
          (pid_nodes, "nodes")
          ("reduce_counter", pid_reduce_counter)
          ;
        tid_t tid_init (map_reduce_subnet.add_transition (trans_init));

        // map
        transition_type trans_map
          ( "map"
          , expr_type
            ( "${output} := ${input};"
              "${node_count} := ${node_count} - 1;"
            )
          , "true"
          , true
          )
          ;

        trans_map.add_port
          ("input", "long", we::type::PORT_IN);
        trans_map.add_port
          ("output", "long", we::type::PORT_OUT)
          ;
        tid_t tid_map (map_reduce_subnet.add_transition (trans_map));

        // edges
        {
          using petri_net::connection_t;
          using petri_net::edge::PT;
          using petri_net::edge::PT_READ;
          using petri_net::edge::TP;

          edge_type e (0);
          map_reduce_subnet.add_edge (e++, connection_t ( PT, tid_init, pid_input ));
          map_reduce_subnet.add_edge (e++, connection_t ( PT, tid_init, pid_nodes ));
          map_reduce_subnet.add_edge (e++, connection_t ( TP, tid_init, pid_node_count ));
        }

        transition_type trans_map_reduce (map_reduce_subnet);
        trans_map_reduce.add_ports
          ("input", "long", we::type::PORT_IN,   pid_input);
        trans_map_reduce.add_ports
          ("nodes", "long", we::type::PORT_IN,   pid_nodes);
        trans_map_reduce.add_ports
          ("output", "long", we::type::PORT_OUT, pid_output)
          ;
        return trans_map_reduce;
      }

      static activity_type generate()
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
            const pid_t pid_wi =
              map_reduce_subnet.add_place(place_t("wi_" + fhg::util::show(n)));
            hull_in.push_back (pid_wi);

            const pid_t pid_wo = map_reduce_subnet.add_place(place_t("wo_" + fhg::util::show(n)));
            hull_out.push_back (pid_wo);

            transition_t wrk_trans ("work"+fhg::util::show(n), typename transition_t::mod_type("map_reduce", "work"), "true");
            wrk_trans.add_port
              ("i", "long", we::type::PORT_IN);
            wrk_trans.add_port
              ("o", "long", we::type::PORT_OUT)
              ;
            wrk_trans.add_connections()
              (pid_wi, "i")
              ("o", pid_wo)
              ;

            const tid_t tid_w = map_reduce_subnet.add_transition (wrk_trans);

            // connect work in
            map_reduce_subnet.add_edge (edge_t("work_in_"+fhg::util::show(n)), petri_net::connection_t (petri_net::edge::PT, tid_w, pid_wi));

            // connect work out
            map_reduce_subnet.add_edge (edge_t("work_out_" + fhg::util::show(n)), petri_net::connection_t (petri_net::edge::TP, tid_w, pid_wo));
          }

          {
            transition_t map_trans ("map", typename transition_t::expr_type("${1} := ${0} + \"-1\"; ${2} := ${0} + \"-2\"; ${3} := ${0} + \"-3\";"), "true", true);
            map_trans.add_port
              ("i", "long", we::type::PORT_IN)
              ;
            map_trans.add_connections()
              (mr_sn_inp, "i")
              ;

            size_t cnt(0);
            for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
            {
              map_trans.add_port ("o"+fhg::util::show (cnt), "long", we::type::PORT_OUT);
              map_trans.add_connections() ("o"+fhg::util::show (cnt), *i);
              cnt++;
            }
            tid_t tid_map = map_reduce_subnet.add_transition ( map_trans );
            map_reduce_subnet.add_edge (edge_t("map"), petri_net::connection_t (petri_net::edge::PT, tid_map, mr_sn_inp));

            cnt = 0;
            for (typename hull_t::const_iterator i = hull_in.begin(); i != hull_in.end(); ++i)
            {
              map_reduce_subnet.add_edge (edge_t("map_" + fhg::util::show(cnt)), petri_net::connection_t (petri_net::edge::TP, tid_map, *i));
              cnt++;
            }
          }

          {
            transition_t red_trans ("red", typename transition_t::expr_type("${1} := substr(${0}, len(\"token-0\"));"), "true", true);

            size_t cnt(0);
            for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
            {
              red_trans.add_port ("i" + fhg::util::show(cnt), "long", we::type::PORT_IN);
              red_trans.add_connections() (*o, "i" + fhg::util::show(cnt));
              cnt++;
            }
            red_trans.add_port ("o", "long", we::type::PORT_OUT);
            red_trans.add_connections() ("o", mr_sn_out);

            tid_t tid_red = map_reduce_subnet.add_transition ( red_trans );
            map_reduce_subnet.add_edge (edge_t("red"), petri_net::connection_t (petri_net::edge::TP, tid_red, mr_sn_out));

            cnt = 0;
            for (typename hull_t::const_iterator o = hull_out.begin(); o != hull_out.end(); ++o)
            {
              map_reduce_subnet.add_edge (edge_t("red_" + fhg::util::show(cnt)), petri_net::connection_t (petri_net::edge::PT, tid_red, *o));
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
          map_reduce_sub_trans.add_port
            ("i", "long", we::type::PORT_IN);  // TODO port_id must be: mr_sn_inp
          map_reduce_sub_trans.add_port
            ("o", "long", we::type::PORT_OUT) // TODO port_id must be: mr_sn_out
            ;
          map_reduce_sub_trans.add_connections ()
            (mr_inp, "i")
            ("o", mr_out)
            ;

          //        map_reduce_sub_trans.connect_in  (mr_inp, mr_sn_inp);
          //        map_reduce_sub_trans.connect_out (mr_out, mr_sn_out);

          tid_t tid_sub = map_reduce.add_transition( map_reduce_sub_trans );
          map_reduce.add_edge (edge_t("i"), petri_net::connection_t (petri_net::edge::PT, tid_sub, mr_inp));
          map_reduce.add_edge (edge_t("o"), petri_net::connection_t (petri_net::edge::TP, tid_sub, mr_out));

          // dummy transition
          transition_t map_reduce_trans ("map-reduce", map_reduce, "true", true);
          map_reduce_trans.add_port
            ("i", "long", we::type::PORT_IN);  // TODO port_id must be mr_inp
          map_reduce_trans.add_port
            ("o", "long", we::type::PORT_OUT) // TODO port_id must be mr_out
            ;
          //        map_reduce_trans.connect_in (pid_t(0), mr_inp);
          //        map_reduce_trans.connect_out (pid_t(1), mr_out);

          activity_type act (map_reduce_trans);

          // put some tokens on the input
          for (std::size_t t (0); t < NUM_TOKEN; ++t)
          {
            act.input().push_back (std::make_pair (token_t ("token-" + fhg::util::show(t)), map_reduce_trans.input_port_by_name ("i")));
          }

          return act;
        }
      };
    }
  }

#endif

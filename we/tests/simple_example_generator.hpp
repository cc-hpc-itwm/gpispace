#ifndef WE_TESTS_SIMPLE_EXAMPLE_HPP
#define WE_TESTS_SIMPLE_EXAMPLE_HPP 1

namespace we { namespace tests { namespace gen {

    template <typename Activity>
    struct simple
    {
      typedef typename Activity::transition_type transition_type;

      typedef typename transition_type::place_type place_type;
      typedef typename transition_type::edge_type edge_type;
      typedef typename transition_type::token_type token_type;

      typedef typename transition_type::net_type net_type;
      typedef typename transition_type::mod_type mod_type;
      typedef typename transition_type::expr_type expr_type;

      typedef typename transition_type::pid_t pid_t;
      typedef typename petri_net::tid_t tid_t;
      typedef typename transition_type::port_id_t port_id_t;

      static transition_type generate ()
      {
        // In:  input (some token)
        // Out: output (some token(s))
        net_type net_subnet ("inner subnet");

        // **************
        // *** places ***
        // **************

        pid_t pid_input  (net_subnet.add_place (place_type ("input", "long")));
        pid_t pid_work   (net_subnet.add_place (place_type ("work",  "long")));
        pid_t pid_output (net_subnet.add_place (place_type ("output", "long")));

        // *******************
        // *** transitions ***
        // *******************

        // increment
        transition_type trans_incr
          ( "incr"
          , expr_type
            ( "${o} := ${i} + 1;" )
          , "true"
          , true
          );
        trans_incr.add_port
          ("i", "long", we::type::PORT_IN);
        trans_incr.add_port
          ("o", "long", we::type::PORT_OUT)
          ;
        trans_incr.add_connections ()
          (pid_input, "i")
          ("o", pid_work)
          ;
        tid_t tid_incr (net_subnet.add_transition (trans_incr));

        // module call
        transition_type trans_mod
          ( "module call"
          , mod_type ("dummy", "dummy")
          );
        trans_mod.add_port
          ("input", "long", we::type::PORT_IN);
        trans_mod.add_port
          ("output", "long", we::type::PORT_OUT)
          ;
        trans_mod.add_connections ()
          (pid_work, "input")
          ("output", pid_output)
          ;
        tid_t tid_mod (net_subnet.add_transition (trans_mod));

        // edges
        {
          using petri_net::connection_t;
          using petri_net::edge::PT;
          using petri_net::edge::PT_READ;
          using petri_net::edge::TP;

          edge_type e (0);
          net_subnet.add_edge (e++, connection_t ( PT, tid_incr, pid_input ));
          net_subnet.add_edge (e++, connection_t ( TP, tid_incr, pid_work ));
          net_subnet.add_edge (e++, connection_t ( PT, tid_mod, pid_work ));
          net_subnet.add_edge (e++, connection_t ( TP, tid_mod, pid_output ));
        }

        // create the inner net
        net_type net_inner ("inner net");
        pid_t pid_i (net_inner.add_place (place_type ("input", "long")));
        pid_t pid_o (net_inner.add_place (place_type ("output", "long")));

        transition_type trans_subnet ("subnet", net_subnet);
        trans_subnet.add_port
          ("input", "long", we::type::PORT_IN, pid_input);
        trans_subnet.add_port
          ("output", "long", we::type::PORT_OUT, pid_output)
          ;
        trans_subnet.add_connections ()
          (pid_i, "input")
          ("output", pid_o)
          ;
        tid_t tid_sub (net_inner.add_transition (trans_subnet));

        // edges
        {
          using petri_net::connection_t;
          using petri_net::edge::PT;
          using petri_net::edge::PT_READ;
          using petri_net::edge::TP;

          edge_type e (0);
          net_inner.add_edge (e++, connection_t ( PT, tid_sub, pid_i ));
          net_inner.add_edge (e++, connection_t ( TP, tid_sub, pid_o ));
        }

        // create the visible transition
        transition_type trans_net ("net", net_inner, "true", transition_type::external);
        trans_net.add_port
          ("input", "long", we::type::PORT_IN,   pid_i);
        trans_net.add_port
          ("output", "long", we::type::PORT_OUT, pid_o)
          ;
        return trans_net;
      }
    };
    }
  }
}

#endif

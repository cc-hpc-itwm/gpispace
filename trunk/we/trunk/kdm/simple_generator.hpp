#ifndef KDM_SIMPLE_GENERATOR_HPP
#define KDM_SIMPLE_GENERATOR_HPP 1

namespace kdm
{
  namespace signature
  {
    static ::signature::structured_t config;
    static ::signature::structured_t state;

    static void init (void)
    {
      config["OFFSETS"] = literal::LONG;
      config["SUBVOLUMES_PER_OFFSET"] = literal::LONG;
      config["BUNCHES_PER_OFFSET"] = literal::LONG;

      state["state"] = literal::LONG;
      state["num"] = literal::LONG;
    }
  }

  using petri_net::connection_t;
  using petri_net::PT;
  using petri_net::PT_READ;
  using petri_net::TP;

  template <typename Activity>
  struct kdm
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

    template<typename T>
    static transition_type
    mk_generate ( const T & sig
                , const ::signature::field_name_t & source
                , const ::signature::field_name_t & name
                , ::signature::structured_t & sig_with_id
                )
    {
      net_type subnet ("generate_" + name + "_sub");

      ::signature::structured_t sig_with_state;
      
      sig_with_state[name] = sig;
      sig_with_state["state"] = signature::state;

      sig_with_id[name] = sig;
      sig_with_id["id"] = literal::LONG;

      pid_t pid_trigger (subnet.add_place (place_type ("trigger", sig)));
      pid_t pid_state (subnet.add_place (place_type ("state", sig_with_state)));
      pid_t pid_name (subnet.add_place (place_type (name, sig_with_id)));
      pid_t pid_config (subnet.add_place (place_type ("config", signature::config)));

      transition_type trans_init
        ( "init"
        , expr_type 
        ( "${state.state.state}  := 0L;"
          "${state.state.num}    := ${config." + source + "};"
          "${state." + name + "} := ${trigger};"
        )
        , "true"
        , transition_type::internal
        );
      trans_init.add_ports ()
        ("config", signature::config, we::type::PORT_IN)
        ("trigger", sig, we::type::PORT_IN)
        ("state", sig_with_state, we::type::PORT_OUT)
        ;
      trans_init.add_connections ()
        (pid_trigger, "trigger")
        (pid_config, "config")
        ("state", pid_state)
        ;
      tid_t tid_init (subnet.add_transition (trans_init));

      transition_type trans_break
        ( "break"
        , expr_type ()
        , "${state.state.state} >= ${state.state.num}"
        , transition_type::internal
        );
      trans_break.add_ports ()
        ("state", sig_with_state, we::type::PORT_IN)
        ;
      trans_break.add_connections ()
        (pid_state, "state")
        ;
      tid_t tid_break (subnet.add_transition (trans_break));

      transition_type trans_step
        ( "step"
        , expr_type 
          ( "${" + name + ".id}           := ${state.state.state};"
            "${" + name + "." + name + "} := ${state." + name + "};"
            "${state.state.state}         := ${state.state.state} + 1"
           )
        , "${state.state.state} < ${state.state.num}"
        , transition_type::internal
        );
      trans_step.add_ports ()
        ("state", sig_with_state, we::type::PORT_IN_OUT)
        (name, sig_with_id, we::type::PORT_OUT)
        ;
      trans_step.add_connections ()
        (pid_state, "state")
        ("state", pid_state)
        (name, pid_name)
        ;
      tid_t tid_step (subnet.add_transition (trans_step));
        
      edge_type e (0);

      subnet.add_edge (e++, connection_t (PT, tid_init, pid_trigger));
      subnet.add_edge (e++, connection_t (PT, tid_init, pid_config));
      subnet.add_edge (e++, connection_t (TP, tid_init, pid_state));
      subnet.add_edge (e++, connection_t (PT, tid_step, pid_state));
      subnet.add_edge (e++, connection_t (TP, tid_step, pid_state));
      subnet.add_edge (e++, connection_t (TP, tid_step, pid_name));
      subnet.add_edge (e++, connection_t (PT, tid_break, pid_state));

      transition_type generate
        ( "generate_" + name
        , subnet
        , "true"
        , transition_type::internal
        );
      generate.add_ports ()
        ("trigger", sig, we::type::PORT_IN, pid_trigger)
        ("config", signature::config, we::type::PORT_IN, pid_config)
        (name + "_with_id", sig_with_id, we::type::PORT_OUT, pid_name)
        ;

      return generate;
    }

    static transition_type generate (void)
    {
      signature::init();

      edge_type e (0);

      net_type net ("test_sub");

      // ******************************************************************* //

      pid_t pid_config_file (net.add_place (place_type ("config_file", literal::STRING)));
      pid_t pid_wait (net.add_place (place_type ("wait", literal::LONG)));
      pid_t pid_trigger_loadTT (net.add_place (place_type ("trigger_loadTT", literal::CONTROL)));
      pid_t pid_trigger_generate_offsets (net.add_place (place_type ("trigger_generate_offsets", literal::CONTROL)));
      pid_t pid_config (net.add_place (place_type ("config", signature::config)));

      // ******************************************************************* //

      transition_type generate_config
        ( "generate_config"
        , expr_type 
          ( "${config.OFFSETS} := 2L;"
            "${config.SUBVOLUMES_PER_OFFSET} := 3L;"
            "${config.BUNCHES_PER_OFFSET} := 5L;"
            "${wait} := ${config.OFFSETS} * ${config.SUBVOLUMES_PER_OFFSET};"
            "${trigger} := [];"
          )
        , "true"
        , transition_type::internal
        );

      generate_config.add_ports ()
        ("config_file", literal::STRING, we::type::PORT_IN)
        ("config", signature::config, we::type::PORT_OUT)
        ("wait", literal::LONG, we::type::PORT_OUT)
        ("trigger", literal::CONTROL, we::type::PORT_OUT)
        ;
      generate_config.add_connections ()
        (pid_config_file, "config_file")
        ("config", pid_config)
        ("wait", pid_wait)
        ("trigger", pid_trigger_loadTT)
        ;
      tid_t tid_generate_config (net.add_transition (generate_config));

      net.add_edge (e++, connection_t (PT, tid_generate_config, pid_config_file));
      net.add_edge (e++, connection_t (TP, tid_generate_config, pid_config));
      net.add_edge (e++, connection_t (TP, tid_generate_config, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_generate_config, pid_trigger_loadTT));

      // ******************************************************************* //
      
      transition_type loadTT
        ( "loadTT"
        , expr_type ("${trigger} := []")
        , "true"
        , transition_type::internal
        );

      loadTT.add_ports ()
        ("config", signature::config, we::type::PORT_IN)
        ("trigger", literal::CONTROL, we::type::PORT_IN_OUT)
        ;
      loadTT.add_connections ()
        (pid_trigger_loadTT, "trigger")
        (pid_config, "config")
        ("trigger", pid_trigger_generate_offsets)
        ;

      tid_t tid_loadTT (net.add_transition (loadTT));

      net.add_edge (e++, connection_t (PT_READ, tid_loadTT, pid_config));
      net.add_edge (e++, connection_t (PT, tid_loadTT, pid_trigger_loadTT));
      net.add_edge (e++, connection_t (TP, tid_loadTT, pid_trigger_generate_offsets));

      // ******************************************************************* //

      ::signature::structured_t control_with_id;

      transition_type generate_offset
        (mk_generate (literal::CONTROL, "OFFSETS", "control", control_with_id));

      pid_t pid_control_with_id (net.add_place (place_type ("control_with_id", control_with_id)));

      generate_offset.add_connections ()
        (pid_trigger_generate_offsets, "trigger")
        (pid_config, "config")
        ("control_with_id", pid_control_with_id)
        ;

      tid_t tid_generate_offset (net.add_transition (generate_offset));

      net.add_edge (e++, connection_t (PT_READ, tid_generate_offset, pid_config));
      net.add_edge (e++, connection_t (PT, tid_generate_offset, pid_trigger_generate_offsets));
      net.add_edge (e++, connection_t (TP, tid_generate_offset, pid_control_with_id));

      pid_t pid_offset (net.add_place (place_type ("offset", literal::LONG)));

      // ******************************************************************* //

      transition_type trans_untag_offset
        ( "untag_offset"
        , expr_type ( "${offset} := ${control_with_id.id}")
        , "true"
        , transition_type::internal
        );
      trans_untag_offset.add_ports ()
        ("control_with_id", control_with_id, we::type::PORT_IN)
        ("offset", literal::LONG, we::type::PORT_OUT)
        ;
      trans_untag_offset.add_connections ()
        (pid_control_with_id, "control_with_id")
        ("offset",pid_offset)
        ;
      tid_t tid_untag_offset (net.add_transition (trans_untag_offset));

      net.add_edge (e++, connection_t (PT, tid_untag_offset, pid_control_with_id));
      net.add_edge (e++, connection_t (TP, tid_untag_offset, pid_offset));

      // ******************************************************************* //

      ::signature::structured_t sig_volume;

      transition_type generate_volume
        ( mk_generate ( literal::LONG
                      , "SUBVOLUMES_PER_OFFSET"
                      , "offset"
                      , sig_volume
                      )
        );

      pid_t pid_volume (net.add_place (place_type ("volume", sig_volume)));

      generate_volume.add_connections ()
        (pid_offset, "trigger")
        (pid_config, "config")
        ("offset_with_id", pid_volume)
        ;

      tid_t tid_generate_volume (net.add_transition (generate_volume));

      net.add_edge (e++, connection_t (PT_READ, tid_generate_volume, pid_config));
      net.add_edge (e++, connection_t (PT, tid_generate_volume, pid_offset));
      net.add_edge (e++, connection_t (TP, tid_generate_volume, pid_volume));

      // ******************************************************************* //

      transition_type trans_net
        ( "trans_net"
        , net
        , "true"
        , transition_type::internal
        );
      trans_net.add_ports ()
        ("config_file", literal::STRING, we::type::PORT_IN, pid_config_file)
        ("volume", sig_volume, we::type::PORT_OUT, pid_volume)
        ("config", signature::config, we::type::PORT_OUT, pid_config)
        ("wait", literal::LONG, we::type::PORT_OUT, pid_wait)
        ;

      return trans_net;
    }
  };
}

#endif

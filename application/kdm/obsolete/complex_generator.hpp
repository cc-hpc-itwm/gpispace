
#ifndef KDM_COMPLEX_GENERATOR_HPP
#define KDM_COMPLEX_GENERATOR_HPP 1

namespace kdm
{
  namespace signature
  {
    static ::signature::structured_t config;
    static ::signature::structured_t state;
    static ::signature::structured_t offset_with_state;
    static ::signature::structured_t bunch;
    static ::signature::structured_t loaded_bunch;
    static ::signature::structured_t buffer;
    static ::signature::structured_t volume;
    static ::signature::structured_t volume_with_buffer;

    static void init (void)
    {
      config["OFFSETS"] = literal::LONG();
      config["BUNCHES_PER_OFFSET"] = literal::LONG();
      config["STORES"] = literal::LONG();
      config["SUBVOLUMES_PER_OFFSET"] = literal::LONG();
      config["PARALLEL_LOADTT"] = literal::LONG();

      config["handle_TT"] = literal::LONG();
      config["handle_Job"] = literal::LONG();
      config["scratch_Job"] = literal::LONG();
      config["handle_Store"] = literal::LONG();
      config["scratch_Store"] = literal::LONG();
      config["handle_Volume"] = literal::LONG();
      config["scratch_Volume"] = literal::LONG();
      config["NThreads"] = literal::LONG();

      state["num"] = literal::LONG();
      state["state"] = literal::LONG();

      offset_with_state["offset"] = literal::LONG();
      offset_with_state["state"] = state;

      bunch["id"] = literal::LONG();
      bunch["offset"] = literal::LONG();

      loaded_bunch["bunch"] = bunch;
      loaded_bunch["store"] = literal::LONG();
      loaded_bunch["seen"] = literal::BITSET();
      loaded_bunch["wait"] = literal::LONG();

      buffer["filled"] = literal::BOOL;
      buffer["assigned"] = literal::BOOL;
      buffer["free"] = literal::BOOL;
      buffer["bunch"] = bunch;
      buffer["store"] = literal::LONG();

      volume["id"] = literal::LONG();
      volume["offset"] = literal::LONG();

      volume_with_buffer["volume"] = volume;
      volume_with_buffer["wait"] = literal::LONG();
      volume_with_buffer["buffer0"] = buffer;
      volume_with_buffer["buffer1"] = buffer;
    }
  }

  namespace value
  {
    namespace bunch
    {
      static ::value::structured_t invalid;

      static void init (void)
      {
        invalid["id"] = -1L;
        invalid["offset"] = -1L;
      }
    }

    namespace buffer
    {
      static ::value::structured_t empty;

      static void init (void)
      {
        empty["assigned"] = false;
        empty["filled"] = false;
        empty["free"] = false;
        empty["store"] = -1L;
        empty["bunch"] = bunch::invalid;
      }
    }

    static void init (void)
    {
      bunch::init();
      buffer::init();
    }
  }

  using petri_net::connection_t;
  using petri_net::PT;
  using petri_net::PT_READ;
  using petri_net::TP;

  template<typename Activity>
  struct kdm
  {
    typedef typename Activity::transition_type transition_type;

    typedef typename transition_type::place_type place_type;
    typedef typename transition_type::edge_type edge_type;
    typedef typename transition_type::token_type token_type;

    typedef typename transition_type::net_type net_type;
    typedef typename transition_type::mod_type mod_type;
    typedef typename transition_type::expr_type expr_type;
    typedef typename transition_type::cond_type cond_type;

    typedef typename transition_type::pid_t pid_t;
    typedef typename petri_net::tid_t tid_t;
    typedef typename transition_type::port_id_t port_id_t;

    static transition_type generate (void)
    {
      signature::init();
      value::init();

      edge_type e (0);

      net_type net ("kdm_complex");

      // ******************************************************************* //

      pid_t pid_config_file (net.add_place (place_type ("config_file", literal::STRING())));
      pid_t pid_config (net.add_place (place_type ("config", signature::config)));
      pid_t pid_trigger_gen_store (net.add_place (place_type ("trigger_gen_store", literal::CONTROL())));
      pid_t pid_trigger_generate_TT (net.add_place (place_type ("trigger_generate_TT", literal::CONTROL())));
      pid_t pid_waitTT (net.add_place (place_type ("waitTT", literal::LONG())));
      pid_t pid_trigger_decTT (net.add_place (place_type ("decTT", literal::CONTROL())));
      pid_t pid_state_generate_TT (net.add_place (place_type ("state_generate_TT", literal::LONG())));
      pid_t pid_TT (net.add_place (place_type ("TT", literal::LONG())));
      pid_t pid_doneTT (net.add_place (place_type ("doneTT", literal::LONG())));
      pid_t pid_gen_store_state (net.add_place (place_type ("gen_store_state", signature::state)));
      pid_t pid_empty_store (net.add_place (place_type ("empty_store", literal::LONG())));
      pid_t pid_trigger_gen_offset (net.add_place (place_type ("trigger_gen_offset", literal::CONTROL())));
      pid_t pid_wanted_offset (net.add_place (place_type ("wanted_offset", literal::BITSET())));
      pid_t pid_gen_offset_state (net.add_place (place_type ("gen_offset_state", signature::state)));
      pid_t pid_offset_bunch (net.add_place (place_type ("offset_bunch", literal::LONG())));
      pid_t pid_offset_volume (net.add_place (place_type ("offset_volume", literal::LONG())));
      pid_t pid_gen_bunch_state (net.add_place (place_type ("gen_bunch_state", signature::offset_with_state)));
      pid_t pid_bunch (net.add_place (place_type ("bunch", signature::bunch)));
      pid_t pid_loaded_bunch (net.add_place (place_type ("loaded_bunch", signature::loaded_bunch)));
      pid_t pid_gen_volume_state (net.add_place (place_type ("gen_volume_state", signature::offset_with_state)));
      pid_t pid_volume (net.add_place (place_type ("volume", signature::volume_with_buffer)));
      pid_t pid_volume_processed (net.add_place (place_type ("volume_processed", signature::volume_with_buffer)));
      pid_t pid_volume_to_be_written (net.add_place (place_type ("volume_to_be_written", signature::volume)));
      pid_t pid_volume_written (net.add_place (place_type ("volume_written", signature::volume)));
      pid_t pid_volume_wait (net.add_place (place_type ("volume_wait", literal::LONG())));
      pid_t pid_buffer_empty (net.add_place (place_type ("buffer_empty", signature::buffer)));
      pid_t pid_assign_xor_reuse_store (net.add_place (place_type ("assign_xor_reuse_store")));
      pid_t pid_done (net.add_place (place_type ("done", literal::CONTROL())));

      token::put (net, pid_buffer_empty, value::buffer::empty);
      token::put (net, pid_assign_xor_reuse_store);

      // ******************************************************************* //

      {
        transition_type initialize
          ( "initialize"
          , mod_type ("kdm_complex", "initialize")
          , transition_type::external
          );
        initialize.add_ports ()
          ("config_file", literal::STRING(), we::type::PORT_IN)
          ("config", signature::config, we::type::PORT_OUT)
          ("trigger", literal::CONTROL(), we::type::PORT_OUT)
          ("wanted", literal::BITSET(), we::type::PORT_OUT)
          ("wait", literal::LONG(), we::type::PORT_OUT)
          ("parallel_loadTT", literal::LONG(), we::type::PORT_OUT)
          ;
        initialize.add_connections ()
          (pid_config_file, "config_file")
          ("config",pid_config)
          ("trigger", pid_trigger_generate_TT)
          ("wanted", pid_wanted_offset)
          ("wait", pid_volume_wait)
          ("parallel_loadTT", pid_waitTT)
          ;
        tid_t tid_initialize (net.add_transition (initialize));

        net.add_edge (e++, connection_t (PT, tid_initialize, pid_config_file));
        net.add_edge (e++, connection_t (TP, tid_initialize, pid_config));
        net.add_edge (e++, connection_t (TP, tid_initialize, pid_trigger_generate_TT));
        net.add_edge (e++, connection_t (TP, tid_initialize, pid_waitTT));
        net.add_edge (e++, connection_t (TP, tid_initialize, pid_wanted_offset));
        net.add_edge (e++, connection_t (TP, tid_initialize, pid_volume_wait));
      }

      // ******************************************************************* //

      {
        transition_type init_genTT ("init_genTT", expr_type ("${TT} := 0L"));
        init_genTT.add_ports ()
          ("TT", literal::LONG(), we::type::PORT_OUT)
          ("trigger", literal::CONTROL(), we::type::PORT_IN)
          ;
        init_genTT.add_connections()
          (pid_trigger_generate_TT, "trigger")
          ("TT", pid_state_generate_TT)
          ;
        tid_t tid_init_genTT (net.add_transition (init_genTT));

        net.add_edge (e++, connection_t (PT, tid_init_genTT, pid_trigger_generate_TT));
        net.add_edge (e++, connection_t (TP, tid_init_genTT, pid_state_generate_TT));
      }

      // ******************************************************************* //

      {
        transition_type stepTT
          ( "stepTT"
          , expr_type ("${TT} := ${state}; ${state} := ${state} + 1")
          , cond_type ("${state} < ${max}")
          );
        stepTT.add_ports ()
          ("TT", literal::LONG(), we::type::PORT_OUT)
          ("state", literal::LONG(), we::type::PORT_IN_OUT)
          ("max", literal::LONG(), we::type::PORT_READ)
          ;
        stepTT.add_connections()
          (pid_state_generate_TT, "state")
          ("state", pid_state_generate_TT)
          ("TT", pid_TT)
          (pid_waitTT, "max")
          ;
        tid_t tid_stepTT (net.add_transition (stepTT));

        net.add_edge (e++, connection_t (PT, tid_stepTT, pid_state_generate_TT));
        net.add_edge (e++, connection_t (PT_READ, tid_stepTT, pid_waitTT));
        net.add_edge (e++, connection_t (TP, tid_stepTT, pid_state_generate_TT));
        net.add_edge (e++, connection_t (TP, tid_stepTT, pid_TT));
      }

      // ******************************************************************* //

      {
        transition_type breakTT
          ( "breakTT"
          , expr_type ("${trigger} := []")
          , cond_type ("${state} >= ${max}")
          );

        breakTT.add_ports ()
          ("state", literal::LONG(), we::type::PORT_IN)
          ("max", literal::LONG(), we::type::PORT_READ)
          ("trigger", literal::CONTROL(), we::type::PORT_OUT)
          ;
        breakTT.add_connections ()
          (pid_waitTT, "max")
          (pid_state_generate_TT, "state")
          ("trigger", pid_trigger_decTT)
          ;

        tid_t tid_breakTT (net.add_transition (breakTT));

        net.add_edge (e++, connection_t (PT, tid_breakTT, pid_state_generate_TT));
        net.add_edge (e++, connection_t (PT_READ, tid_breakTT, pid_waitTT));
        net.add_edge (e++, connection_t (TP, tid_breakTT, pid_trigger_decTT));
      }

      // ******************************************************************* //

      {
        transition_type loadTT ("loadTT", mod_type ("kdm_complex", "loadTT"));
        loadTT.add_ports ()
          ("TT", literal::LONG(), we::type::PORT_IN_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ;
        loadTT.add_connections ()
          (pid_TT, "TT")
          ("TT", pid_doneTT)
          (pid_config, "config")
          ;
        tid_t tid_loadTT (net.add_transition (loadTT));

        net.add_edge (e++, connection_t (PT, tid_loadTT, pid_TT));
        net.add_edge (e++, connection_t (TP, tid_loadTT, pid_doneTT));
        net.add_edge (e++, connection_t (PT_READ, tid_loadTT, pid_config));
      }

      // ******************************************************************* //

      {
        transition_type decTT ("decTT", expr_type ("${wait} := ${wait} - 1"));
        decTT.add_ports ()
          ("wait", literal::LONG(), we::type::PORT_IN_OUT)
          ("doneTT", literal::LONG(), we::type::PORT_IN)
          ("trigger", literal::CONTROL(), we::type::PORT_READ)
          ;
        decTT.add_connections()
          ("wait", pid_waitTT)
          (pid_waitTT, "wait")
          (pid_doneTT, "doneTT")
          (pid_trigger_decTT, "trigger")
          ;
        tid_t tid_decTT (net.add_transition (decTT));

        net.add_edge (e++, connection_t (PT, tid_decTT, pid_waitTT));
        net.add_edge (e++, connection_t (PT, tid_decTT, pid_doneTT));
        net.add_edge (e++, connection_t (PT_READ, tid_decTT, pid_trigger_decTT));
        net.add_edge (e++, connection_t (TP, tid_decTT, pid_waitTT));
      }

      // ******************************************************************* //

      {
        transition_type finalize_generate_TT
          ( "finalize_generate_TT"
          , expr_type ("${trigger} := []")
          , "${wait} == 0L"
          );
        finalize_generate_TT.add_ports()
          ("trigger", literal::CONTROL(), we::type::PORT_IN_OUT)
          ("wait", literal::LONG(), we::type::PORT_IN)
          ;
        finalize_generate_TT.add_connections()
          (pid_waitTT, "wait")
          ("trigger", pid_trigger_gen_store)
          (pid_trigger_decTT, "trigger")
          ;
        tid_t tid_finalize_generate_TT (net.add_transition (finalize_generate_TT));

        net.add_edge (e++, connection_t (PT, tid_finalize_generate_TT, pid_waitTT));
        net.add_edge (e++, connection_t (PT, tid_finalize_generate_TT, pid_trigger_decTT));
        net.add_edge (e++, connection_t (TP, tid_finalize_generate_TT, pid_trigger_gen_store));
      }

      // ******************************************************************* //

      {
        transition_type gen_store
          ( "gen_store"
          , expr_type
            ( "${state.num} := ${config.STORES};"
              "${state.state} := 0L"
            )
          );
        gen_store.add_ports ()
          ("config", signature::config, we::type::PORT_READ)
          ("trigger", literal::LONG(), we::type::PORT_IN)
          ("state", signature::state, we::type::PORT_OUT)
          ;
        gen_store.add_connections()
          (pid_trigger_gen_store, "trigger")
          (pid_config, "config")
          ("state", pid_gen_store_state)
          ;

        tid_t tid_gen_store (net.add_transition (gen_store));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_store, pid_config));
        net.add_edge (e++, connection_t (PT, tid_gen_store, pid_trigger_gen_store));
        net.add_edge (e++, connection_t (TP, tid_gen_store, pid_gen_store_state));
      }

      // ******************************************************************* //

      {
        transition_type gen_store_step
          ( "gen_store_step"
          , expr_type
            ( "${empty_store} := ${state.state};"
              "${state.state} := ${state.state} + 1"
            )
          , cond_type ("${state.state} < ${state.num}")
          );
        gen_store_step.add_ports ()
          ("empty_store", literal::LONG(), we::type::PORT_OUT)
          ("state", signature::state, we::type::PORT_IN_OUT)
          ;
        gen_store_step.add_connections ()
          (pid_gen_store_state, "state")
          ("state", pid_gen_store_state)
          ("empty_store", pid_empty_store)
          ;
        tid_t tid_gen_store_step (net.add_transition (gen_store_step));

        net.add_edge (e++, connection_t (PT, tid_gen_store_step, pid_gen_store_state));
        net.add_edge (e++, connection_t (TP, tid_gen_store_step, pid_gen_store_state));
        net.add_edge (e++, connection_t (TP, tid_gen_store_step, pid_empty_store));
      }

      // ******************************************************************* //

      {
        transition_type gen_store_break
          ( "gen_store_break"
          , expr_type ("${trigger} := []")
          , cond_type ("${state.state} >= ${state.num}")
          );
        gen_store_break.add_ports ()
          ("trigger", literal::CONTROL(), we::type::PORT_OUT)
          ("state", signature::state, we::type::PORT_IN)
          ;
        gen_store_break.add_connections ()
          ("trigger", pid_trigger_gen_offset)
          (pid_gen_store_state, "state")
          ;

        tid_t tid_gen_store_break (net.add_transition (gen_store_break));

        net.add_edge (e++, connection_t (PT, tid_gen_store_break, pid_gen_store_state));
        net.add_edge (e++, connection_t (TP, tid_gen_store_break, pid_trigger_gen_offset));
      }

      // ********************************************************************* //

      {
        transition_type gen_offset
          ( "gen_offset"
          , expr_type
            ( "${state.num} := ${config.OFFSETS};"
              "${state.state} := 0L"
            )
          );
        gen_offset.add_ports ()
          ("config", signature::config, we::type::PORT_READ)
          ("trigger", literal::CONTROL(), we::type::PORT_IN)
          ("state", signature::state, we::type::PORT_OUT)
          ;
        gen_offset.add_connections ()
          (pid_config, "config")
          (pid_trigger_gen_offset, "trigger")
          ("state", pid_gen_offset_state)
          ;
        tid_t tid_gen_offset (net.add_transition (gen_offset));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_offset, pid_config));
        net.add_edge (e++, connection_t (PT, tid_gen_offset, pid_trigger_gen_offset));
        net.add_edge (e++, connection_t (TP, tid_gen_offset, pid_gen_offset_state));
      }

      // ******************************************************************* //

      {
        transition_type gen_offset_step
          ( "gen_offset_step"
          , expr_type
            ( "${offsetA} := ${state.state};"
              "${offsetB} := ${state.state};"
              "${state.state} := ${state.state} + 1"
            )
          , cond_type
            ( "${state.state} < ${state.num} &"
              "bitset_is_element (${wanted}, ${state.state})"
            )
          );
        gen_offset_step.add_ports ()
          ("state", signature::state, we::type::PORT_IN_OUT)
          ("offsetA", literal::LONG(), we::type::PORT_OUT)
          ("offsetB", literal::LONG(), we::type::PORT_OUT)
          ("wanted", literal::BITSET(), we::type::PORT_READ)
          ;
        gen_offset_step.add_connections ()
          ("state", pid_gen_offset_state)
          (pid_gen_offset_state, "state")
          ("offsetA", pid_offset_bunch)
          ("offsetB", pid_offset_volume)
          (pid_wanted_offset, "wanted")
          ;

        tid_t tid_gen_offset_step (net.add_transition (gen_offset_step));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_offset_step, pid_wanted_offset));
        net.add_edge (e++, connection_t (PT, tid_gen_offset_step, pid_gen_offset_state));
        net.add_edge (e++, connection_t (TP, tid_gen_offset_step, pid_gen_offset_state));
        net.add_edge (e++, connection_t (TP, tid_gen_offset_step, pid_offset_bunch));
        net.add_edge (e++, connection_t (TP, tid_gen_offset_step, pid_offset_volume));
      }

      // ******************************************************************* //

      {
        transition_type gen_offset_break
          ( "gen_offset_break"
          , expr_type ()
          , cond_type ("${state.state} >= ${state.num}")
          );
        gen_offset_break.add_ports ()
          ("state", signature::state, we::type::PORT_IN)
          ;
        gen_offset_break.add_connections ()
          (pid_gen_offset_state, "state")
          ;

        tid_t tid_gen_offset_break (net.add_transition (gen_offset_break));

        net.add_edge (e++, connection_t (PT, tid_gen_offset_break, pid_gen_offset_state));
      }

      // ********************************************************************* //

      {
        transition_type gen_bunch
          ( "gen_bunch"
          , expr_type
            ( "${state.state.num} := ${config.BUNCHES_PER_OFFSET};"
              "${state.state.state} := 0L;"
              "${state.offset} := ${offset}"
            )
          );
        gen_bunch.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_OUT)
          ("offset", literal::LONG(), we::type::PORT_IN)
          ("config", signature::config, we::type::PORT_READ)
          ;
        gen_bunch.add_connections ()
          (pid_config, "config")
          (pid_offset_bunch, "offset")
          ("state", pid_gen_bunch_state)
          ;
        tid_t tid_gen_bunch (net.add_transition (gen_bunch));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_bunch, pid_config));
        net.add_edge (e++, connection_t (PT, tid_gen_bunch, pid_offset_bunch));
        net.add_edge (e++, connection_t (TP, tid_gen_bunch, pid_gen_bunch_state));
      }

      // ******************************************************************* //

      {
        transition_type gen_bunch_step
          ( "gen_bunch_step"
          , expr_type
            ( "${bunch.id} := ${state.state.state};"
              "${bunch.offset} := ${state.offset};"
              "${state.state.state} := ${state.state.state} + 1"
            )
          , cond_type ("${state.state.state} < ${state.state.num}")
          );
        gen_bunch_step.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_IN_OUT)
          ("bunch", signature::bunch, we::type::PORT_OUT)
          ;
        gen_bunch_step.add_connections ()
          (pid_gen_bunch_state, "state")
          ("state", pid_gen_bunch_state)
          ("bunch", pid_bunch)
          ;

        tid_t tid_gen_bunch_step (net.add_transition (gen_bunch_step));

        net.add_edge (e++, connection_t (PT, tid_gen_bunch_step, pid_gen_bunch_state));
        net.add_edge (e++, connection_t (TP, tid_gen_bunch_step, pid_gen_bunch_state));
        net.add_edge (e++, connection_t (TP, tid_gen_bunch_step, pid_bunch));
      }

      // ******************************************************************* //

      {
        transition_type gen_bunch_break
          ( "gen_bunch_break"
          , expr_type ()
          , cond_type ("${state.state.state} >= ${state.state.num}")
          );
        gen_bunch_break.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_IN)
          ;
        gen_bunch_break.add_connections ()
          (pid_gen_bunch_state, "state")
          ;

        tid_t tid_gen_bunch_break (net.add_transition (gen_bunch_break));

        net.add_edge (e++, connection_t (PT, tid_gen_bunch_break, pid_gen_bunch_state));
      }

      // ********************************************************************* //

      {
        transition_type gen_volume
          ( "gen_volume"
          , expr_type
            ( "${state.state.state} := 0L;"
              "${state.state.num} := ${config.SUBVOLUMES_PER_OFFSET};"
              "${state.offset} := ${offset}"
            )
          );
        gen_volume.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ("offset", literal::LONG(), we::type::PORT_IN)
          ;
        gen_volume.add_connections ()
          ("state", pid_gen_volume_state)
          (pid_config, "config")
          (pid_offset_volume, "offset")
          ;

        tid_t tid_gen_volume (net.add_transition (gen_volume));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_volume, pid_config));
        net.add_edge (e++, connection_t (PT, tid_gen_volume, pid_offset_volume));
        net.add_edge (e++, connection_t (TP, tid_gen_volume, pid_gen_volume_state));
      }

      // ******************************************************************* //

      {
        transition_type gen_volume_step
          ( "gen_volume_step"
          , expr_type
            ( "${volume.volume.id} := ${state.state.state};"
              "${volume.volume.offset} := ${state.offset};"
              "${volume.wait} := ${config.BUNCHES_PER_OFFSET};"
              "${volume.buffer0} := ${buffer_empty};"
              "${volume.buffer1} := ${buffer_empty};"
              "${state.state.state} := ${state.state.state} + 1"
            )
          , cond_type ("${state.state.state} < ${state.state.num}")
          );
        gen_volume_step.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_IN_OUT)
          ("volume", signature::volume_with_buffer, we::type::PORT_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ("buffer_empty", signature::buffer, we::type::PORT_READ)
          ;
        gen_volume_step.add_connections ()
          ("state", pid_gen_volume_state)
          (pid_gen_volume_state, "state")
          ("volume", pid_volume)
          (pid_config, "config")
          (pid_buffer_empty, "buffer_empty")
          ;

        tid_t tid_gen_volume_step (net.add_transition (gen_volume_step));

        net.add_edge (e++, connection_t (PT_READ, tid_gen_volume_step, pid_config));
        net.add_edge (e++, connection_t (PT_READ, tid_gen_volume_step, pid_buffer_empty));
        net.add_edge (e++, connection_t (PT, tid_gen_volume_step, pid_gen_volume_state));
        net.add_edge (e++, connection_t (TP, tid_gen_volume_step, pid_gen_volume_state));
        net.add_edge (e++, connection_t (TP, tid_gen_volume_step, pid_volume));
      }

      // ******************************************************************* //

      {
        transition_type gen_volume_break
          ( "gen_volume_break"
          , expr_type ()
          , cond_type ("${state.state.state} >= ${state.state.num}")
          );
        gen_volume_break.add_ports ()
          ("state", signature::offset_with_state, we::type::PORT_IN)
          ;
        gen_volume_break.add_connections ()
          (pid_gen_volume_state, "state")
          ;

        tid_t tid_gen_volume_break (net.add_transition (gen_volume_break));

        net.add_edge (e++, connection_t (PT, tid_gen_volume_break, pid_gen_volume_state));
      }

      // ********************************************************************* //

      {
        transition_type load ("load", mod_type ("kdm_complex", "load"));
        load.add_ports ()
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_OUT)
          ("bunch", signature::bunch, we::type::PORT_IN)
          ("empty_store", literal::LONG(), we::type::PORT_IN)
          ("config", signature::config, we::type::PORT_READ)
          ;
        load.add_connections ()
          ("loaded_bunch", pid_loaded_bunch)
          (pid_empty_store, "empty_store")
          (pid_bunch, "bunch")
          (pid_config, "config")
          ;

        tid_t tid_load (net.add_transition (load));

        net.add_edge (e++, connection_t (PT_READ, tid_load, pid_config));
        net.add_edge (e++, connection_t (PT, tid_load, pid_empty_store));
        net.add_edge (e++, connection_t (PT, tid_load, pid_bunch));
        net.add_edge (e++, connection_t (TP, tid_load, pid_loaded_bunch));
      }

      // ******************************************************************* //

      {
        transition_type reuse_store
          ( "reuse_store"
          , expr_type ("${empty_store} := ${loaded_bunch.store}")
          , cond_type ("${loaded_bunch.wait} == 0L")
          );
        reuse_store.add_ports ()
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_IN)
          ("empty_store", literal::LONG(), we::type::PORT_OUT)
          ;
        reuse_store.add_connections ()
          (pid_loaded_bunch, "loaded_bunch")
          ("empty_store", pid_empty_store)
          ;

        tid_t tid_reuse_store (net.add_transition (reuse_store));

        net.add_edge (e++, connection_t (PT, tid_reuse_store, pid_loaded_bunch));
        net.add_edge (e++, connection_t (TP, tid_reuse_store, pid_empty_store));
      }

      // ********************************************************************* //

      {
        transition_type assign0
          ( "assign0"
          , expr_type
            ( "${loaded_bunch.seen} := bitset_insert (${loaded_bunch.seen}, ${volume.volume.id});"
              "${volume.buffer0.assigned} := true;"
              "${volume.buffer0.bunch} := ${loaded_bunch.bunch};"
              "${volume.buffer0.store} := ${loaded_bunch.store};"
              "${trigger} := []"
            )
          , cond_type
            ( "(!${volume.buffer0.assigned}) &"
              "(${volume.volume.offset} == ${loaded_bunch.bunch.offset}) &"
              "(!bitset_is_element (${loaded_bunch.seen}, ${volume.volume.id}))"
            )
          );
        assign0.add_ports ()
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_IN_OUT)
          ("volume", signature::volume_with_buffer, we::type::PORT_IN_OUT)
          ("trigger", literal::CONTROL(), we::type::PORT_IN_OUT)
          ;
        assign0.add_connections()
          ("loaded_bunch", pid_loaded_bunch)
          (pid_loaded_bunch, "loaded_bunch")
          ("volume", pid_volume)
          (pid_volume, "volume")
          ("trigger", pid_assign_xor_reuse_store)
          (pid_assign_xor_reuse_store, "trigger")
          ;
        tid_t tid_assign0 (net.add_transition (assign0));

        net.add_edge (e++, connection_t (PT, tid_assign0, pid_loaded_bunch));
        net.add_edge (e++, connection_t (TP, tid_assign0, pid_loaded_bunch));
        net.add_edge (e++, connection_t (PT, tid_assign0, pid_volume));
        net.add_edge (e++, connection_t (TP, tid_assign0, pid_volume));
        net.add_edge (e++, connection_t (PT, tid_assign0, pid_assign_xor_reuse_store));
        net.add_edge (e++, connection_t (TP, tid_assign0, pid_assign_xor_reuse_store));
        net.set_transition_priority (tid_assign0, 1);
      }

      // ******************************************************************* //

      {
        transition_type assign1
          ( "assign1"
          , expr_type
            ( "${loaded_bunch.seen} := bitset_insert (${loaded_bunch.seen}, ${volume.volume.id});"
              "${volume.buffer1.assigned} := true;"
              "${volume.buffer1.bunch} := ${loaded_bunch.bunch};"
              "${volume.buffer1.store} := ${loaded_bunch.store};"
              "${trigger} := []"
            )
          , cond_type
            ( "(!${volume.buffer1.assigned}) &"
              "(${volume.volume.offset} == ${loaded_bunch.bunch.offset}) &"
              "(!bitset_is_element (${loaded_bunch.seen}, ${volume.volume.id}))"
            )
          );
        assign1.add_ports ()
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_IN_OUT)
          ("volume", signature::volume_with_buffer, we::type::PORT_IN_OUT)
          ("trigger", literal::CONTROL(), we::type::PORT_IN_OUT)
          ;
        assign1.add_connections()
          ("loaded_bunch", pid_loaded_bunch)
          (pid_loaded_bunch, "loaded_bunch")
          ("volume", pid_volume)
          (pid_volume, "volume")
          ("trigger", pid_assign_xor_reuse_store)
          (pid_assign_xor_reuse_store, "trigger")
          ;
        tid_t tid_assign1 (net.add_transition (assign1));

        net.add_edge (e++, connection_t (PT, tid_assign1, pid_loaded_bunch));
        net.add_edge (e++, connection_t (TP, tid_assign1, pid_loaded_bunch));
        net.add_edge (e++, connection_t (PT, tid_assign1, pid_volume));
        net.add_edge (e++, connection_t (TP, tid_assign1, pid_volume));
        net.add_edge (e++, connection_t (PT, tid_assign1, pid_assign_xor_reuse_store));
        net.add_edge (e++, connection_t (TP, tid_assign1, pid_assign_xor_reuse_store));
        net.set_transition_priority (tid_assign1, 1);
      }

      // ********************************************************************* //

      {
        transition_type process
          ( "process"
          , mod_type ("kdm_complex", "process")
          , cond_type ("${volume.buffer0.assigned} | ${volume.buffer1.assigned}")
          );
        process.add_ports ()
          ("volume", signature::volume_with_buffer, we::type::PORT_IN)
          ("volume_processed", signature::volume_with_buffer, we::type::PORT_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ;
        process.add_connections ()
          (pid_volume, "volume")
          ("volume_processed", pid_volume_processed)
          (pid_config, "config")
          ;

        tid_t tid_process (net.add_transition (process));

        net.add_edge (e++, connection_t (PT_READ, tid_process, pid_config));
        net.add_edge (e++, connection_t (PT, tid_process, pid_volume));
        net.add_edge (e++, connection_t (TP, tid_process, pid_volume_processed));
      }

      // ********************************************************************* //

      {
        transition_type reuse_store0
          ( "reuse_store0"
          , expr_type
            ( "${loaded_bunch.wait} := ${loaded_bunch.wait} - 1;"
              "${volume.buffer0.free} := false;"
              "${trigger} := []"
            )
          , cond_type
            ( "(${volume.buffer0.bunch} == ${loaded_bunch.bunch}) &"
              "(${volume.buffer0.free})"
            )
          );
        reuse_store0.add_ports ()
          ("trigger", literal::CONTROL(), we::type::PORT_IN_OUT)
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_IN_OUT)
          ("volume", signature::volume_with_buffer, we::type::PORT_IN_OUT)
          ;
        reuse_store0.add_connections ()
          (pid_loaded_bunch, "loaded_bunch")
          ("loaded_bunch", pid_loaded_bunch)
          (pid_volume_processed, "volume")
          ("volume", pid_volume_processed)
          (pid_assign_xor_reuse_store, "trigger")
          ("trigger", pid_assign_xor_reuse_store)
          ;

        tid_t tid_reuse_store0 (net.add_transition (reuse_store0));

        net.add_edge (e++, connection_t (PT, tid_reuse_store0, pid_loaded_bunch));
        net.add_edge (e++, connection_t (TP, tid_reuse_store0, pid_loaded_bunch));
        net.add_edge (e++, connection_t (PT, tid_reuse_store0, pid_volume_processed));
        net.add_edge (e++, connection_t (TP, tid_reuse_store0, pid_volume_processed));
        net.add_edge (e++, connection_t (PT, tid_reuse_store0, pid_assign_xor_reuse_store));
        net.add_edge (e++, connection_t (TP, tid_reuse_store0, pid_assign_xor_reuse_store));
        net.set_transition_priority (tid_reuse_store0, 1);
      }

      // ******************************************************************* //

      {
        transition_type reuse_store1
          ( "reuse_store1"
          , expr_type ("${loaded_bunch.wait} := ${loaded_bunch.wait} - 1;\
         ${volume.buffer1.free} := false;\
         ${trigger} := []"
                      )
          , cond_type ("(${volume.buffer1.bunch} == ${loaded_bunch.bunch}) &\
         (${volume.buffer1.free})")
          );
        reuse_store1.add_ports ()
          ("trigger", literal::CONTROL(), we::type::PORT_IN_OUT)
          ("loaded_bunch", signature::loaded_bunch, we::type::PORT_IN_OUT)
          ("volume", signature::volume_with_buffer, we::type::PORT_IN_OUT)
          ;
        reuse_store1.add_connections ()
          (pid_loaded_bunch, "loaded_bunch")
          ("loaded_bunch", pid_loaded_bunch)
          (pid_volume_processed, "volume")
          ("volume", pid_volume_processed)
          (pid_assign_xor_reuse_store, "trigger")
          ("trigger", pid_assign_xor_reuse_store)
          ;

        tid_t tid_reuse_store1 (net.add_transition (reuse_store1));

        net.add_edge (e++, connection_t (PT, tid_reuse_store1, pid_loaded_bunch));
        net.add_edge (e++, connection_t (TP, tid_reuse_store1, pid_loaded_bunch));
        net.add_edge (e++, connection_t (PT, tid_reuse_store1, pid_volume_processed));
        net.add_edge (e++, connection_t (TP, tid_reuse_store1, pid_volume_processed));
        net.add_edge (e++, connection_t (PT, tid_reuse_store1, pid_assign_xor_reuse_store));
        net.add_edge (e++, connection_t (TP, tid_reuse_store1, pid_assign_xor_reuse_store));
        net.set_transition_priority (tid_reuse_store1, 1);
      }

      // ********************************************************************* //

      {
        transition_type volume_step
          ( "volume_step"
          , expr_type ("${volume} := ${volume_processed}")
          , cond_type ("${volume_processed.wait} > 0L")
          );
        volume_step.add_ports ()
          ("trigger", literal::CONTROL(), we::type::PORT_READ)
          ("volume", signature::volume_with_buffer, we::type::PORT_OUT)
          ("volume_processed", signature::volume_with_buffer, we::type::PORT_IN)
          ;
        volume_step.add_connections ()
          (pid_assign_xor_reuse_store, "trigger")
          (pid_volume_processed, "volume_processed")
          ("volume", pid_volume)
          ;
        tid_t tid_volume_step (net.add_transition (volume_step));

        net.add_edge (e++, connection_t (PT, tid_volume_step, pid_volume_processed));
        net.add_edge (e++, connection_t (TP, tid_volume_step, pid_volume));
        net.add_edge (e++, connection_t (PT_READ, tid_volume_step, pid_assign_xor_reuse_store));
      }
      // ******************************************************************* //

      {
        transition_type volume_break
          ( "volume_break"
          , expr_type ("${volume} := ${volume_processed.volume}")
          , cond_type ("${volume_processed.wait} == 0L")
          );
        volume_break.add_ports ()
          ("volume_processed", signature::volume_with_buffer, we::type::PORT_IN)
          ("volume", signature::volume, we::type::PORT_OUT)
          ("trigger", literal::LONG(), we::type::PORT_READ)
          ;
        volume_break.add_connections ()
          (pid_volume_processed, "volume_processed")
          ("volume", pid_volume_to_be_written)
          (pid_assign_xor_reuse_store, "trigger")
          ;

        tid_t tid_volume_break (net.add_transition (volume_break));

        net.add_edge (e++, connection_t (PT, tid_volume_break, pid_volume_processed));
        net.add_edge (e++, connection_t (TP, tid_volume_break, pid_volume_to_be_written));
        net.add_edge (e++, connection_t (PT_READ, tid_volume_break, pid_assign_xor_reuse_store));
      }

      // ******************************************************************* //

      {
        transition_type write ("write", mod_type ("kdm_complex", "write"));
        write.add_ports ()
          ("volume", signature::volume, we::type::PORT_IN_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ;
        write.add_connections ()
          (pid_volume_to_be_written, "volume")
          ("volume", pid_volume_written)
          (pid_config, "config")
          ;

        tid_t tid_write (net.add_transition (write));

        net.add_edge (e++, connection_t (PT_READ, tid_write, pid_config));
        net.add_edge (e++, connection_t (PT, tid_write, pid_volume_to_be_written));
        net.add_edge (e++, connection_t (TP, tid_write, pid_volume_written));
      }

      // ******************************************************************* //

      {
        transition_type volume_next_offset
          ( "volume_next_offset"
          , expr_type
            ( "${wanted_offset} := "
              "bitset_insert (${wanted_offset}, ${volume_written.offset} + 1)"
            )
          , cond_type ("${volume_written.offset} + 1 < ${config.OFFSETS}")
          );
        volume_next_offset.add_ports()
          ("config", signature::config, we::type::PORT_READ)
          ("wanted_offset", literal::BITSET(), we::type::PORT_IN_OUT)
          ("volume_written", signature::volume, we::type::PORT_IN)
          ;
        volume_next_offset.add_connections()
          (pid_config, "config")
          (pid_wanted_offset, "wanted_offset")
          ("wanted_offset", pid_wanted_offset)
          (pid_volume_written, "volume_written")
          ;

        tid_t tid_volume_next_offset (net.add_transition (volume_next_offset));

        net.add_edge (e++, connection_t (PT_READ, tid_volume_next_offset, pid_config));
        net.add_edge (e++, connection_t (PT, tid_volume_next_offset, pid_wanted_offset));
        net.add_edge (e++, connection_t (TP, tid_volume_next_offset, pid_wanted_offset));
        net.add_edge (e++, connection_t (PT, tid_volume_next_offset, pid_volume_written));
      }

      // ******************************************************************* //

      {
        transition_type volume_done
          ( "volume_done"
          , expr_type ("${volume_wait} := ${volume_wait} - 1")
          , cond_type ("${volume_written.offset} + 1 >= ${config.OFFSETS}")
          );
        volume_done.add_ports ()
          ("volume_wait", literal::LONG(), we::type::PORT_IN_OUT)
          ("config", signature::config, we::type::PORT_READ)
          ("volume_written", signature::volume, we::type::PORT_IN)
          ;
        volume_done.add_connections ()
          ("volume_wait", pid_volume_wait)
          (pid_volume_wait, "volume_wait")
          (pid_config, "config")
          (pid_volume_written, "volume_written")
          ;
        tid_t tid_volume_done (net.add_transition (volume_done));

        net.add_edge (e++, connection_t (PT_READ, tid_volume_done, pid_config));
        net.add_edge (e++, connection_t (PT, tid_volume_done, pid_volume_written));
        net.add_edge (e++, connection_t (PT, tid_volume_done, pid_volume_wait));
        net.add_edge (e++, connection_t (TP, tid_volume_done, pid_volume_wait));
      }

      // ********************************************************************* //

      {
        transition_type finalize
          ( "finalize"
          , mod_type ("kdm_complex", "finalize")
          , cond_type ("${volume_wait} == 0L")
          );
        finalize.add_ports ()
          ("done", literal::CONTROL(), we::type::PORT_OUT)
          ("volume_wait", literal::LONG(), we::type::PORT_IN)
          ("config", signature::config, we::type::PORT_IN)
          ;
        finalize.add_connections ()
          ("done", pid_done)
          (pid_config, "config")
          (pid_volume_wait, "volume_wait")
          ;

        tid_t tid_finalize (net.add_transition (finalize));

        net.add_edge (e++, connection_t (PT, tid_finalize, pid_volume_wait));
        net.add_edge (e++, connection_t (PT, tid_finalize, pid_config));
        net.add_edge (e++, connection_t (TP, tid_finalize, pid_done));
      }

      // *********************************************************************** //

      transition_type trans_net ( "kdm_complex"
                                , net
                                , transition_type::external
                                );
      trans_net.add_ports ()
        ("config_file", literal::STRING(), we::type::PORT_IN, pid_config_file)
        ("done", literal::CONTROL(), we::type::PORT_OUT, pid_done)
        ;

      return trans_net;
    }
  };
}

#endif

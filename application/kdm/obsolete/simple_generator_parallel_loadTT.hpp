#ifndef KDM_SIMPLE_GENERATOR_PARALLEL_LOADTT_HPP
#define KDM_SIMPLE_GENERATOR_PARALLEL_LOADTT_HPP 1

namespace kdm
{
  namespace signature
  {
    static ::signature::structured_t config;
    static ::signature::structured_t state;

    static void init (void)
    {
      config["OFFSETS"] = literal::LONG();
      config["SUBVOLUMES_PER_OFFSET"] = literal::LONG();
      config["BUNCHES_PER_OFFSET"] = literal::LONG();
      config["PARALLEL_LOADTT"] = literal::LONG();

      config["handle_Job"] = literal::LONG();
      config["scratch_Job"] = literal::LONG();
      config["handle_TT"] = literal::LONG();
      config["NThreads"] = literal::LONG();

      state["state"] = literal::LONG();
      state["num"] = literal::LONG();
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
    typedef typename transition_type::cond_type cond_type;

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
      sig_with_id["id"] = literal::LONG();

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
        , cond_type ("${state.state.state} >= ${state.state.num}")
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
        , cond_type ("${state.state.state} < ${state.state.num}")
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
        ( "generate_from_" + name
        , subnet
        , transition_type::internal
        );
      generate.add_ports ()
        ("trigger", sig, we::type::PORT_IN, pid_trigger)
        ("config", signature::config, we::type::PORT_READ, pid_config)
        (name + "_with_id", sig_with_id, we::type::PORT_OUT, pid_name)
        ;

      return generate;
    }

    template<typename T>
    static transition_type mk_dup (const T & sig)
    {
      transition_type dup
        ( "dup"
        , expr_type
          ( "${one} := ${in};"
            "${two} := ${in};"
          )
        );

      dup.add_ports ()
        ("in", sig, we::type::PORT_IN)
        ("one", sig, we::type::PORT_OUT)
        ("two", sig, we::type::PORT_OUT)
        ;

      return dup;
    }

    static transition_type
    mk_process ( const ::signature::structured_t & sig_volume
               , const ::signature::structured_t & sig_config
               )
    {
      edge_type e (0);

      net_type net ("process_volume");

      // ******************************************************************* //

      pid_t pid_volume_in (net.add_place (place_type ("volume_in", sig_volume)));
      pid_t pid_volume_initialized (net.add_place (place_type ("volume_initialized", sig_volume)));
      pid_t pid_volume_gen_bunch (net.add_place (place_type ("volume_gen_bunch", sig_volume)));
      pid_t pid_volume (net.add_place (place_type ("volume", sig_volume)));
      pid_t pid_config_in (net.add_place (place_type ("config_in", sig_config)));
      pid_t pid_config (net.add_place (place_type ("config", sig_config)));
      pid_t pid_wait (net.add_place (place_type ("wait", literal::LONG())));
      pid_t pid_done (net.add_place (place_type ("done", literal::CONTROL())));
      pid_t pid_trigger_load (net.add_place (place_type ("trigger_load", literal::CONTROL())));

      token::put (net, pid_trigger_load);

      // ******************************************************************* //

      transition_type init
        ( "init_volume"
        , mod_type ("kdm_parallel_loadTT", "init_volume")
        );

      init.add_ports ()
        ("volume", sig_volume, we::type::PORT_IN_OUT)
        ("config", sig_config, we::type::PORT_READ)
        ;

      init.add_connections ()
        ("volume", pid_volume_initialized)
        (pid_volume_in, "volume")
        (pid_config, "config")
        ;

      tid_t tid_init (net.add_transition (init));

      net.add_edge (e++, connection_t (PT, tid_init, pid_volume_in));
      net.add_edge (e++, connection_t (PT_READ, tid_init, pid_config));
      net.add_edge (e++, connection_t (TP, tid_init, pid_volume_initialized));

      // ******************************************************************* //

      transition_type dup (mk_dup (sig_volume));

      dup.add_connections ()
        (pid_volume_initialized, "in")
        ("one", pid_volume)
        ("two", pid_volume_gen_bunch)
        ;

      tid_t tid_dup (net.add_transition (dup));

      net.add_edge (e++, connection_t (PT, tid_dup, pid_volume_initialized));
      net.add_edge (e++, connection_t (TP, tid_dup, pid_volume));
      net.add_edge (e++, connection_t (TP, tid_dup, pid_volume_gen_bunch));

      // ******************************************************************* //

      transition_type init_wait
        ( "init_wait"
        , expr_type ("${num} := ${config.BUNCHES_PER_OFFSET}")
        );

      init_wait.add_ports ()
        ("config", sig_config, we::type::PORT_IN_OUT)
        ("num", literal::LONG(), we::type::PORT_OUT)
        ;

      init_wait.add_connections()
        (pid_config_in, "config")
        ("config", pid_config)
        ("num", pid_wait)
        ;

      tid_t tid_init_wait (net.add_transition (init_wait));

      net.add_edge (e++, connection_t (PT, tid_init_wait, pid_config_in));
      net.add_edge (e++, connection_t (TP, tid_init_wait, pid_config));
      net.add_edge (e++, connection_t (TP, tid_init_wait, pid_wait));

      // ******************************************************************* //

      ::signature::structured_t sig_bunch;

      transition_type generate_bunch
        ( mk_generate ( sig_volume
                      , "BUNCHES_PER_OFFSET"
                      , "volume"
                      , sig_bunch
                      )
        );

      pid_t pid_bunch (net.add_place (place_type ("bunch", sig_bunch)));

      generate_bunch.add_connections ()
        (pid_volume_gen_bunch, "trigger")
        (pid_config, "config")
        ("volume_with_id", pid_bunch)
        ;

      tid_t tid_generate_bunch (net.add_transition (generate_bunch));

      net.add_edge (e++, connection_t (PT_READ, tid_generate_bunch, pid_config));
      net.add_edge (e++, connection_t (PT, tid_generate_bunch, pid_volume_gen_bunch));
      net.add_edge (e++, connection_t (TP, tid_generate_bunch, pid_bunch));

      // ******************************************************************* //

      pid_t pid_trigger_process
        (net.add_place (place_type ("trigger_process", sig_bunch)));

      transition_type load
        ( "load"
        , mod_type ("kdm_parallel_loadTT", "load")
        );

      load.add_ports ()
        ("config", sig_config, we::type::PORT_READ)
        ("bunch", sig_bunch, we::type::PORT_IN_OUT)
        ("trigger", literal::CONTROL(), we::type::PORT_IN)
        ;
      load.add_connections ()
        (pid_bunch, "bunch")
        ("bunch", pid_trigger_process)
        (pid_config, "config")
        (pid_trigger_load, "trigger")
        ;

      tid_t tid_load (net.add_transition (load));

      net.add_edge (e++, connection_t (PT_READ, tid_load, pid_config));
      net.add_edge (e++, connection_t (PT, tid_load, pid_bunch));
      net.add_edge (e++, connection_t (PT, tid_load, pid_trigger_load));
      net.add_edge (e++, connection_t (TP, tid_load, pid_trigger_process));

      // ******************************************************************* //

      transition_type process
        ( "process"
        , mod_type ("kdm_parallel_loadTT", "process")
        );

      process.add_ports ()
        ("config", sig_config, we::type::PORT_READ)
        ("bunch", sig_bunch, we::type::PORT_IN)
        ("wait", literal::LONG(), we::type::PORT_IN_OUT)
        ("trigger", literal::CONTROL(), we::type::PORT_OUT)
        ;

      process.add_connections ()
        (pid_trigger_process, "bunch")
        (pid_config, "config")
        ("wait", pid_wait)
        (pid_wait, "wait")
        ("trigger", pid_trigger_load)
        ;

      tid_t tid_process (net.add_transition (process));

      net.add_edge (e++, connection_t (PT_READ, tid_process, pid_config));
      net.add_edge (e++, connection_t (PT, tid_process, pid_trigger_process));
      net.add_edge (e++, connection_t (PT, tid_process, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_process, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_process, pid_trigger_load));

      // ******************************************************************* //

      transition_type write
        ( "write"
        , mod_type ("kdm_parallel_loadTT", "write")
        , cond_type ("${wait} == 0L")
        );

      write.add_ports ()
        ("config", sig_config, we::type::PORT_IN)
        ("wait", literal::LONG(), we::type::PORT_IN)
        ("volume", sig_volume, we::type::PORT_IN)
        ("done", literal::CONTROL(), we::type::PORT_OUT)
        ;

      write.add_connections ()
        (pid_config, "config")
        (pid_wait, "wait")
        (pid_volume, "volume")
        ("done", pid_done)
        ;

      tid_t tid_write (net.add_transition (write));

      net.add_edge (e++, connection_t (PT, tid_write, pid_config));
      net.add_edge (e++, connection_t (PT, tid_write, pid_volume));
      net.add_edge (e++, connection_t (PT, tid_write, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_write, pid_done));

      // ******************************************************************* //

      transition_type process_volume
        ( "process_volume"
        , net
        , transition_type::external
        );

      process_volume.add_ports ()
        ("volume", sig_volume, we::type::PORT_IN, pid_volume_in)
        ("config", sig_config, we::type::PORT_IN, pid_config_in)
        ("done", literal::CONTROL(), we::type::PORT_OUT, pid_done)
        ;

      // ******************************************************************* //

      net_type wrap ("wrap");

      pid_t pid_wrap_volume (wrap.add_place (place_type ("volume", sig_volume)));
      pid_t pid_wrap_config (wrap.add_place (place_type ("config", sig_config)));
      pid_t pid_wrap_done (wrap.add_place (place_type ("done", literal::CONTROL())));

      process_volume.add_connections ()
        (pid_wrap_volume, "volume")
        (pid_wrap_config, "config")
        ("done", pid_wrap_done)
        ;

      tid_t tid_wrapped_proc (wrap.add_transition (process_volume));

      wrap.add_edge (e++, connection_t (PT, tid_wrapped_proc, pid_wrap_volume));
      wrap.add_edge (e++, connection_t (PT, tid_wrapped_proc, pid_wrap_config));
      wrap.add_edge (e++, connection_t (TP, tid_wrapped_proc, pid_wrap_done));

      transition_type process_volume_wrapped
        ( "process_volume_wrapped"
        , wrap
        , transition_type::external
        );

      process_volume_wrapped.add_ports ()
        ("volume", sig_volume, we::type::PORT_IN, pid_wrap_volume)
        ("config", sig_config, we::type::PORT_IN, pid_wrap_config)
        ("done", literal::CONTROL(), we::type::PORT_OUT, pid_wrap_done)
        ;

      return process_volume_wrapped;
    }

    static transition_type generate (void)
    {
      signature::init();

      edge_type e (0);

      net_type net ("test_sub");

      // ******************************************************************* //

      pid_t pid_config_file (net.add_place (place_type ("config_file", literal::STRING())));
      pid_t pid_wait (net.add_place (place_type ("wait", literal::LONG())));
      pid_t pid_trigger_generate_TT (net.add_place (place_type ("trigger_generate_TT", literal::CONTROL())));
      pid_t pid_waitTT (net.add_place (place_type ("waitTT", literal::LONG())));
      pid_t pid_TT (net.add_place (place_type ("TT", literal::LONG())));
      pid_t pid_doneTT (net.add_place (place_type ("doneTT", literal::LONG())));
      pid_t pid_trigger_generate_offsets (net.add_place (place_type ("trigger_generate_offsets", literal::CONTROL())));
      pid_t pid_trigger_dec (net.add_place (place_type ("trigger_dec", literal::CONTROL())));
      pid_t pid_done (net.add_place (place_type ("done", literal::CONTROL())));
      pid_t pid_config (net.add_place (place_type ("config", signature::config)));

      // ******************************************************************* //

      transition_type initialize
        ( "initialize"
        , mod_type ("kdm_parallel_loadTT", "initialize")
        );

      initialize.add_ports ()
        ("config_file", literal::STRING(), we::type::PORT_IN)
        ("config", signature::config, we::type::PORT_OUT)
        ("wait", literal::LONG(), we::type::PORT_OUT)
        ("parallel_loadTT", literal::LONG(), we::type::PORT_OUT)
        ("trigger", literal::CONTROL(), we::type::PORT_OUT)
        ;
      initialize.add_connections ()
        (pid_config_file, "config_file")
        ("config", pid_config)
        ("wait", pid_wait)
        ("parallel_loadTT", pid_waitTT)
        ("trigger", pid_trigger_generate_TT)
        ;
      tid_t tid_initialize (net.add_transition (initialize));

      net.add_edge (e++, connection_t (PT, tid_initialize, pid_config_file));
      net.add_edge (e++, connection_t (TP, tid_initialize, pid_config));
      net.add_edge (e++, connection_t (TP, tid_initialize, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_initialize, pid_trigger_generate_TT));
      net.add_edge (e++, connection_t (TP, tid_initialize, pid_waitTT));

      // ******************************************************************* //

      ::signature::structured_t control_with_TT;

      transition_type generate_TT (mk_generate ( literal::CONTROL()
                                               , "PARALLEL_LOADTT"
                                               , "TT_control"
                                               , control_with_TT
                                               )
                                  );

      pid_t pid_control_with_TT (net.add_place (place_type ("control_with_TT"
                                                           , control_with_TT
                                                           )
                                               )
                                );

      generate_TT.add_connections ()
        (pid_trigger_generate_TT, "trigger")
        (pid_config, "config")
        ("TT_control_with_id", pid_control_with_TT)
        ;

      tid_t tid_generate_TT (net.add_transition (generate_TT));

      net.add_edge (e++, connection_t (PT_READ, tid_generate_TT, pid_config));
      net.add_edge (e++, connection_t (PT, tid_generate_TT, pid_trigger_generate_TT));
      net.add_edge (e++, connection_t (TP, tid_generate_TT, pid_control_with_TT));

      // ******************************************************************* //

      transition_type trans_untag_TT
        ( "untag_TT"
        , expr_type ( "${TT} := ${control_with_TT.id}")
        );
      trans_untag_TT.add_ports ()
        ("control_with_TT", control_with_TT, we::type::PORT_IN)
        ("TT", literal::LONG(), we::type::PORT_OUT)
        ;
      trans_untag_TT.add_connections ()
        (pid_control_with_TT, "control_with_TT")
        ("TT",pid_TT)
        ;
      tid_t tid_untag_TT (net.add_transition (trans_untag_TT));

      net.add_edge (e++, connection_t (PT, tid_untag_TT, pid_control_with_TT));
      net.add_edge (e++, connection_t (TP, tid_untag_TT, pid_TT));

      // ******************************************************************* //

      transition_type loadTT
        ( "loadTT"
        , mod_type ("kdm_parallel_loadTT","loadTT")
        );

      loadTT.add_ports ()
        ("config", signature::config, we::type::PORT_READ)
        ("TT", literal::LONG(), we::type::PORT_IN_OUT)
        ;
      loadTT.add_connections ()
        (pid_TT, "TT")
        (pid_config, "config")
        ("TT", pid_doneTT)
        ;

      tid_t tid_loadTT (net.add_transition (loadTT));

      net.add_edge (e++, connection_t (PT_READ, tid_loadTT, pid_config));
      net.add_edge (e++, connection_t (PT, tid_loadTT, pid_TT));
      net.add_edge (e++, connection_t (TP, tid_loadTT, pid_doneTT));

      // ******************************************************************* //

      transition_type decTT ("decTT", expr_type ("${wait} := ${wait} - 1"));

      decTT.add_ports ()
        ("trigger", literal::LONG(), we::type::PORT_IN)
        ("wait", literal::LONG(), we::type::PORT_IN_OUT)
        ;
      decTT.add_connections ()
        (pid_doneTT, "trigger")
        (pid_waitTT, "wait")
        ("wait", pid_waitTT)
        ;

      tid_t tid_decTT (net.add_transition (decTT));

      net.add_edge (e++, connection_t (PT, tid_decTT, pid_doneTT));
      net.add_edge (e++, connection_t (PT, tid_decTT, pid_waitTT));
      net.add_edge (e++, connection_t (TP, tid_decTT, pid_waitTT));

      // ******************************************************************* //

      transition_type finTT
        ( "finTT"
        , expr_type ("${trigger} := []")
        , cond_type ("${wait} == 0L")
        );

      finTT.add_ports ()
        ("wait", literal::LONG(), we::type::PORT_IN)
        ("trigger", literal::CONTROL(), we::type::PORT_OUT)
        ;
      finTT.add_connections ()
        (pid_waitTT, "wait")
        ("trigger", pid_trigger_generate_offsets)
        ;

      tid_t tid_finTT (net.add_transition (finTT));

      net.add_edge (e++, connection_t (PT, tid_finTT, pid_waitTT));
      net.add_edge (e++, connection_t (TP, tid_finTT, pid_trigger_generate_offsets));

      // ******************************************************************* //

      ::signature::structured_t control_with_offset;

      transition_type generate_offset (mk_generate ( literal::CONTROL()
                                                   , "OFFSETS"
                                                   , "OFF_control"
                                                   , control_with_offset
                                                   )
                                      );

      pid_t pid_control_with_offset (net.add_place (place_type ("control_with_offset", control_with_offset)));

      generate_offset.add_connections ()
        (pid_trigger_generate_offsets, "trigger")
        (pid_config, "config")
        ("OFF_control_with_id", pid_control_with_offset)
        ;

      tid_t tid_generate_offset (net.add_transition (generate_offset));

      net.add_edge (e++, connection_t (PT_READ, tid_generate_offset, pid_config));
      net.add_edge (e++, connection_t (PT, tid_generate_offset, pid_trigger_generate_offsets));
      net.add_edge (e++, connection_t (TP, tid_generate_offset, pid_control_with_offset));

      pid_t pid_offset (net.add_place (place_type ("offset", literal::LONG())));

      // ******************************************************************* //

      transition_type trans_untag_offset
        ( "untag_offset"
        , expr_type ( "${offset} := ${control_with_offset.id}")
        );
      trans_untag_offset.add_ports ()
        ("control_with_offset", control_with_offset, we::type::PORT_IN)
        ("offset", literal::LONG(), we::type::PORT_OUT)
        ;
      trans_untag_offset.add_connections ()
        (pid_control_with_offset, "control_with_offset")
        ("offset",pid_offset)
        ;
      tid_t tid_untag_offset (net.add_transition (trans_untag_offset));

      net.add_edge (e++, connection_t (PT, tid_untag_offset, pid_control_with_offset));
      net.add_edge (e++, connection_t (TP, tid_untag_offset, pid_offset));

      // ******************************************************************* //

      ::signature::structured_t sig_volume;

      transition_type generate_volume
        ( mk_generate ( literal::LONG()
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

#undef SERIALIZE_PROC
#ifdef SERIALIZE_PROC
      pid_t pid_serialize_proc (net.add_place (place_type ("serialize_proc", literal::CONTROL())));
      pid_t pid_done_proc (net.add_place (place_type ("done_proc", literal::CONTROL())));
      pid_t pid_volume_to_process (net.add_place (place_type ("volume_to_process", sig_volume)));

      token::put (net, pid_serialize_proc);

      transition_type start_process
        ( "start_proc"
        , expr_type ("${out} := ${in}")
        );

      start_process.add_ports ()
        ("in", sig_volume, we::type::PORT_IN)
        ("out", sig_volume, we::type::PORT_OUT)
        ("trigger", literal::CONTROL(), we::type::PORT_IN)
        ;
      start_process.add_connections ()
        (pid_volume, "in")
        (pid_serialize_proc, "trigger")
        ("out", pid_volume_to_process)
        ;
      tid_t tid_start_process (net.add_transition (start_process));

      net.add_edge (e++, connection_t (PT, tid_start_process, pid_volume));
      net.add_edge (e++, connection_t (PT, tid_start_process, pid_serialize_proc));
      net.add_edge (e++, connection_t (TP, tid_start_process, pid_volume_to_process));
#endif

      transition_type process_volume
        (mk_process (sig_volume, signature::config));

      process_volume.add_connections ()
#ifdef SERIALIZE_PROC
        (pid_volume_to_process, "volume")
        ("done", pid_done_proc)
#else
        (pid_volume, "volume")
        ("done", pid_trigger_dec)
#endif
        (pid_config, "config")
        ;

      tid_t tid_process_volume (net.add_transition (process_volume));

      net.add_edge (e++, connection_t (PT_READ, tid_process_volume, pid_config));
#ifdef SERIALIZE_PROC
      net.add_edge (e++, connection_t (PT, tid_process_volume, pid_volume_to_process));
      net.add_edge (e++, connection_t (TP, tid_process_volume, pid_done_proc));
#else
      net.add_edge (e++, connection_t (PT, tid_process_volume, pid_volume));
      net.add_edge (e++, connection_t (TP, tid_process_volume, pid_trigger_dec));
#endif

#ifdef SERIALIZE_PROC
      transition_type done_proc
        ( "done_proc"
        , expr_type ("${a} := ${i}; ${b} := ${i}")
        );
      done_proc.add_ports ()
        ("i", literal::CONTROL(), we::type::PORT_IN)
        ("a", literal::CONTROL(), we::type::PORT_OUT)
        ("b", literal::CONTROL(), we::type::PORT_OUT)
        ;
      done_proc.add_connections ()
        (pid_done_proc, "i")
        ("a", pid_trigger_dec)
        ("b", pid_serialize_proc)
        ;
      tid_t tid_done_proc (net.add_transition (done_proc));

      net.add_edge (e++, connection_t (PT, tid_done_proc, pid_done_proc));
      net.add_edge (e++, connection_t (TP, tid_done_proc, pid_trigger_dec));
      net.add_edge (e++, connection_t (TP, tid_done_proc, pid_serialize_proc));
#endif

      // ******************************************************************* //

      transition_type dec
        ( "dec"
        , expr_type ("${wait} := ${wait} - 1")
        );

      dec.add_ports ()
        ("trigger", literal::CONTROL(), we::type::PORT_IN)
        ("wait", literal::LONG(), we::type::PORT_IN_OUT)
        ;
      dec.add_connections ()
        (pid_trigger_dec, "trigger")
        (pid_wait, "wait")
        ("wait", pid_wait)
        ;

      tid_t tid_dec (net.add_transition (dec));

      net.add_edge (e++, connection_t (PT, tid_dec, pid_trigger_dec));
      net.add_edge (e++, connection_t (PT, tid_dec, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_dec, pid_wait));

      // ******************************************************************* //

      transition_type finalize
        ( "finalize"
        , mod_type ("kdm_parallel_loadTT", "finalize")
        , cond_type ("${wait} == 0L")
        );

      finalize.add_ports ()
        ("wait", literal::LONG(), we::type::PORT_IN)
        ("config", signature::config, we::type::PORT_IN)
        ("trigger", literal::CONTROL(), we::type::PORT_OUT)
        ;
      finalize.add_connections ()
        (pid_wait, "wait")
        (pid_config, "config")
        ("trigger", pid_done)
        ;

      tid_t tid_finalize (net.add_transition (finalize));

      net.add_edge (e++, connection_t (PT, tid_finalize, pid_config));
      net.add_edge (e++, connection_t (PT, tid_finalize, pid_wait));
      net.add_edge (e++, connection_t (TP, tid_finalize, pid_done));

      // ******************************************************************* //

      transition_type trans_net
        ( "trans_net"
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

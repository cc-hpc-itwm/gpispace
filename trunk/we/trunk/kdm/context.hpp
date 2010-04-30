#ifndef WE_KDM_EXEC_CONTEXT_HPP
#define WE_KDM_EXEC_CONTEXT_HPP 1

struct exec_context
{
  typedef we::transition_t::net_type net_t;
  typedef we::transition_t::mod_type mod_t;
  typedef we::transition_t::expr_type expr_t;

  void handle_internally ( we::activity_t & act, net_t &)
  {
    act.inject_input ();

    while (act.has_enabled())
    {
      we::activity_t sub = act.extract ();
      sub.execute (*this);
      act.inject (sub);
    }

    act.collect_output ();
  }

  void handle_internally ( we::activity_t & act, const mod_t & mod)
  {
    module::call ( act, mod );
  }

  void handle_internally ( we::activity_t & , const expr_t & )
  {
    // nothing to do
  }

  std::string fake_external ( const std::string & act_enc, net_t & n )
  {
    we::activity_t act = we::util::text_codec::decode<we::activity_t> (act_enc);
    handle_internally ( act, n );
    return we::util::text_codec::encode (act);
  }

  void handle_externally ( we::activity_t & act, net_t & n)
  {
    we::activity_t result ( we::util::text_codec::decode<we::activity_t> (fake_external (we::util::text_codec::encode(act), n)));
    act = result;
  }

  std::string fake_external ( const std::string & act_enc, const mod_t & mod )
  {
    we::activity_t act = we::util::text_codec::decode<we::activity_t> (act_enc);
    module::call ( act, mod );
    return we::util::text_codec::encode (act);
  }

  void handle_externally ( we::activity_t & act, const mod_t & module_call )
  {
    we::activity_t result ( we::util::text_codec::decode<we::activity_t> (fake_external (we::util::text_codec::encode(act), module_call)));
    act = result;
  }

  void handle_externally ( we::activity_t & act, const expr_t & e)
  {
    handle_internally ( act, e );
  }
};

#endif

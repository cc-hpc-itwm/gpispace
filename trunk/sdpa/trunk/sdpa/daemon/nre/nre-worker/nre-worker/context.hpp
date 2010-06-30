#ifndef SDPA_DAEMON_NRE_EXEC_CONTEXT_HPP
#define SDPA_DAEMON_NRE_EXEC_CONTEXT_HPP 1

#include <we/loader/loader.hpp>
#include <we/mgmt/context.hpp>

struct exec_context : public we::mgmt::context<>
{
  typedef we::transition_t::net_type net_t;
  typedef we::transition_t::mod_type mod_t;
  typedef we::transition_t::expr_type expr_t;

  exec_context (we::loader::loader & module_loader)
    : loader (module_loader)
  { }

  void handle_internally ( we::activity_t & act, net_t &)
  {
    act.inject_input ();

    while (act.has_enabled())
    {
      we::activity_t sub = act.extract ();
	  sub.inject_input ();
      sub.execute (*this);
      act.inject (sub);
    }

    act.collect_output ();
  }

  void handle_internally ( we::activity_t & act, const mod_t & mod)
  {
    module::call ( loader, act, mod );
  }

  void handle_internally ( we::activity_t & , const expr_t & )
  {
    // nothing to do
  }

  void handle_externally ( we::activity_t & act, net_t & n)
  {
    handle_internally (act, n);
  }

  void handle_externally ( we::activity_t & act, const mod_t & module_call )
  {
    handle_internally (act, module_call);
  }

  void handle_externally ( we::activity_t & act, const expr_t & e)
  {
    handle_internally ( act, e );
  }

private:
  we::loader::loader & loader;
};

#endif

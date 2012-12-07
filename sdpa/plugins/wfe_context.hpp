#ifndef WFE_PLUGIN_CONTEXT_HPP
#define WFE_PLUGIN_CONTEXT_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>

#include <we/type/module_call.hpp>

#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include "wfe_task.hpp"

struct wfe_exec_context : public we::mgmt::context<>
{
  typedef petri_net::net net_t;
  typedef we::type::module_call_t mod_t;
  typedef we::type::expression_t expr_t;

  wfe_exec_context (we::loader::loader & module_loader, wfe_task_t & target)
    : loader (module_loader)
    , task (target)
  {}

  void handle_internally ( we::activity_t & act, net_t &)
  {
    act.inject_input ();

    while (act.can_fire() && (task.state != wfe_task_t::CANCELED))
    {
      we::activity_t sub (act.extract ());
      sub.inject_input ();
      sub.execute (*this);
      act.inject (sub);
    }

    act.collect_output ();
  }

  void handle_internally ( we::activity_t & act, const mod_t & mod)
  {
    try
    {
      module::call (loader, act, mod);
    }
    catch (std::exception const &ex)
    {
      throw std::runtime_error
        ( "call to '" + mod.module () + "::" + mod.function () + "'"
        + " failed: " + ex.what ()
        );
    }
  }

  void handle_internally ( we::activity_t & , const expr_t &)
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
  wfe_task_t & task;
};

#endif

#ifndef WFE_PLUGIN_CONTEXT_HPP
#define WFE_PLUGIN_CONTEXT_HPP 1

#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>

#include <we/type/module_call.hpp>

#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include "wfe_task.hpp"

struct wfe_exec_context : public we::mgmt::context
{
  wfe_exec_context (we::loader::loader& module_loader, wfe_task_t& target)
    : loader (module_loader)
    , task (target)
  {}

  virtual int handle_internally (we::mgmt::type::activity_t& act, net_t &)
  {
    act.inject_input();

    while (act.can_fire() && (task.state != wfe_task_t::CANCELED))
    {
      we::mgmt::type::activity_t sub (act.extract());
      sub.inject_input();
      sub.execute (this);
      act.inject (sub);
    }

    act.collect_output();

    return 0;
  }

  virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
  {
    try
    {
      module::call (loader, act, mod);
    }
    catch (std::exception const &ex)
    {
      throw std::runtime_error
        ( "call to '" + mod.module() + "::" + mod.function() + "'"
        + " failed: " + ex.what()
        );
    }

    return 0;
  }

  virtual int handle_internally (we::mgmt::type::activity_t&, expr_t&)
  {
    return 0;
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
  {
    return handle_internally (act, n);
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& module_call)
  {
    return handle_internally (act, module_call);
  }

  virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
  {
    return handle_internally (act, e);
  }

private:
  we::loader::loader& loader;
  wfe_task_t& task;
};

#endif

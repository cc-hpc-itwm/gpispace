#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/module_call.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <drts/private/scoped_allocation.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <functional>

namespace we
{
  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/*
      , gspc::scoped_allocation /*const*/*
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      , std::function<void (char const*, sdpa::daemon::NotificationEvent::state_t)> const& emit_gantt
      );
  }
}

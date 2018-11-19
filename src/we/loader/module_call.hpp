#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/module_call.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <drts/private/scoped_allocation.hpp>
#include <drts/cache_management/cache_manager.hpp>

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
      , drts::cache::cache_manager* cache
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      , std::function < void ( sdpa::daemon::NotificationEvent::type_t
                             , sdpa::daemon::NotificationEvent::state_t
                             )
                      > const& emit_gantt
      );
  }
}

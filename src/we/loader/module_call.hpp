#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/activity.hpp>
#include <we/type/module_call.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <drts/private/scoped_allocation.hpp>

#include <sdpa/daemon/NotificationService.hpp>

namespace we
{
  namespace loader
  {
    void module_call ( we::loader::loader& loader
                     , gpi::pc::client::api_t /*const*/*
                     , gspc::scoped_allocation /*const*/*
                     , drts::worker::context* context
                     , we::type::activity_t& act
                     , const we::type::module_call_t& module_call
      , sdpa::daemon::NotificationService* service
      , std::string const& worker_name
      , std::string const& activity_id
                     );
  }
}

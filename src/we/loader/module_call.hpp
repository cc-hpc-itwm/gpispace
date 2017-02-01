#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/module_call.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <drts/private/scoped_vmem_cache.hpp>

namespace we
{
  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/*
      , gspc::scoped_vmem_cache*
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      );
  }
}

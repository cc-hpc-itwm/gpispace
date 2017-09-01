#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/module_call.hpp>

#include <drts/private/scoped_vmem_cache.hpp>

#include <vmem/ipc_client.hpp>

namespace we
{
  namespace loader
  {
    expr::eval::context module_call
      ( we::loader::loader& loader
      , intertwine::vmem::ipc_client*
      , boost::optional<intertwine::vmem::shared_cache_id_t>
      , gspc::scoped_vmem_cache const*
      , drts::worker::context* context
      , expr::eval::context const& input
      , const we::type::module_call_t& module_call
      );
  }
}

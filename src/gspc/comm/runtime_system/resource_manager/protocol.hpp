#pragma once

#include <gspc/Forest.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>
#include <gspc/rpc/TODO.hpp>

namespace gspc
{
  namespace comm
  {
    namespace runtime_system
    {
      namespace resource_manager
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( add
          , void (Forest<resource::ID, resource::Class>)
          );
        FHG_RPC_FUNCTION_DESCRIPTION
          ( remove
          , void (Forest<resource::ID>)
          );
      }
    }
  }
}

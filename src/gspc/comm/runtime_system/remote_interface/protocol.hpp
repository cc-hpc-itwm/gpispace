#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Resource.hpp>
#include <gspc/resource/ID.hpp>
#include <gspc/rpc/TODO.hpp>

#include <tuple>

namespace gspc
{
  namespace comm
  {
    namespace runtime_system
    {
      namespace remote_interface
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( add
          , UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
              (UniqueForest<Resource>)
          );

        FHG_RPC_FUNCTION_DESCRIPTION
          ( remove
          , Forest<resource::ID, ErrorOr<>> (Forest<resource::ID>)
          );

        FHG_RPC_FUNCTION_DESCRIPTION
          ( worker_endpoint_for_scheduler
          , rpc::endpoint (resource::ID)
          );
      }
    }
  }
}

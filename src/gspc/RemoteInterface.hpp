#pragma once

#include <gspc/Tree.hpp>
#include <gspc/ResourceManager.hpp>

#include <util-generic/hard_integral_typedef.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace gspc
{
  namespace remote_interface
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (ID, std::uint64_t);

    using Hostname = std::string;
    using Strategy = std::string;
  }

  struct RemoteInterface
  {
    TreeResult<resource::ID> add
      ( resource_manager::ID
      , TreeResult<resource::ID>
      );
    TreeResult<resource::ID> add
      ( resource_manager::ID
      , Tree<resource::ID>
      );

    std::unordered_map< resource_manager::ID
                      , Forest<MaybeError<resource::ID>>
                      >
      const& resources() const;

    //! blocks if there are active resources
    ~RemoteInterface();

  private:
    std::unordered_map< resource_manager::ID
                      , Forest<MaybeError<resource::ID>>
                      > _resources;
  };
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::remote_interface::ID);

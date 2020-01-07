#pragma once

#include <gspc/MaybeError.hpp>
#include <gspc/Resource.hpp>
#include <gspc/Tree.hpp>

#include <util-generic/hard_integral_typedef.hpp>

#include <cstdint>
#include <functional>

namespace gspc
{
  namespace resource_manager
  {
    FHG_UTIL_HARD_INTEGRAL_TYPEDEF (ID, std::uint64_t);
  }

  struct ResourceManager
  {
    Forest<MaybeError<resource::ID>>> add (Forest<resource::ID>);

    resource::ID add (Args&&...); // create Resource
    Resource const& at (resource::ID);

    resource::ID aquire (resource::Type);
    void release (resource::ID);

    std::unordered_map<resource>
  };

  namespace resource_manager
  {
    using Connection = std::reference_wrapper<ResourceManager>;
  }
}

FHG_UTIL_HARD_INTEGRAL_TYPEDEF_HASH (gspc::resource_manager::ID);

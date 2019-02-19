#pragma once

#include <util-generic/warning.hpp>

#include <list>
#include <memory>
#include <vector>

namespace gspc
{
  namespace stencil_cache
  {
    using Coordinate = long;
    using Stencil = long;
  }
}

#define GSPC_STENCIL_CACHE_CALLBACK(_ret,_name)         \
  DISABLE_WARNING_CLANG("-Wreturn-type-c-linkage")      \
  extern "C" [[gnu::visibility ("default")]]            \
    _ret gspc_stencil_cache_callback_ ## _name          \
  RESTORE_WARNING_CLANG("-Wreturn-type-c-linkage")

GSPC_STENCIL_CACHE_CALLBACK (std::shared_ptr<void>, init)
  ( std::vector<char> const&
  );

GSPC_STENCIL_CACHE_CALLBACK
  (std::list<gspc::stencil_cache::Coordinate>, neighbors)
  ( void*
  , gspc::stencil_cache::Stencil
  );

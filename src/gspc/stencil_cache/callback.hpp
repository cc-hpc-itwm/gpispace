#pragma once

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

#if defined(__clang__)
  #define DISABLE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE                     \
    _Pragma ("clang diagnostic push")                                     \
    _Pragma ("clang diagnostic ignored \"-Wreturn-type-c-linkage\"")
  #define RESTORE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE                     \
    _Pragma ("clang diagnostic pop")
#else
  #define DISABLE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE
  #define RESTORE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE
#endif

#define GSPC_STENCIL_CACHE_CALLBACK(_ret,_name)         \
  DISABLE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE           \
  extern "C" [[gnu::visibility ("default")]]            \
    _ret gspc_stencil_cache_callback_ ## _name          \
  RESTORE_WARNING_CLANG_RETURN_TYPE_C_LINKAGE

GSPC_STENCIL_CACHE_CALLBACK (std::shared_ptr<void>, init)
  ( std::vector<char> const&
  );

GSPC_STENCIL_CACHE_CALLBACK
  (std::list<gspc::stencil_cache::Coordinate>, neighbors)
  ( void*
  , gspc::stencil_cache::Stencil
  );

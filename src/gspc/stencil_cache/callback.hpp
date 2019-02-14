#pragma once

#include <gspc/stencil_cache/types.hpp>

#include <list>
#include <memory>
#include <vector>

#define GSPC_STENCIL_CACHE_CALLBACK(_ret,_name) \
  extern "C" [[gnu::visibility ("default")]]    \
    _ret gspc_stencil_cache_callback_ ## _name

GSPC_STENCIL_CACHE_CALLBACK (std::shared_ptr<void>, init)
  ( std::vector<char> const&
  );

GSPC_STENCIL_CACHE_CALLBACK
  (std::list<gspc::stencil_cache::Coordinate>, neighbors)
  ( void*
  , gspc::stencil_cache::Stencil
  );
